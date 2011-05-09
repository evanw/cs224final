#include "mainwindow.h"
#include "document.h"
#include "ui_mainwindow.h"
#include "meshconstruction.h"
#include "catmullclark.h"
#include "meshevolution.h"
#include "convexhull3d.h"
#include "edgefairing.h"
#include "trianglestoquads.h"
#include "meshsculpter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#define WINDOW_TITLE "cs224final"
#define DIALOG_FILE_FILTER "*.obj"
#define FILE_EXTENSION ".obj"
#define SETTINGS_NAME "cs224final"
#define SETTING_DIRECTORY "file_dialog_dir"
#define UPDATE_UNDO_REDO QEvent::User

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QActionGroup *group = new QActionGroup(this);
    group->addAction(ui->actionAddJoints);
    group->addAction(ui->actionScaleJoints);
    group->addAction(ui->actionEditMesh);
    group->addAction(ui->actionSculptMesh);
    group->addAction(ui->actionAnimate);

    ui->mirrorChanges->setChecked(true);
    ui->drawWireframe->setChecked(true);
    ui->drawInterpolated->setChecked(true);
    ui->orbitCamera->setChecked(true);
    ui->brushAddOrSubtract->setChecked(true);

    ui->brushRadius->setRange(0, 100);
    ui->brushWeight->setRange(0, 100);
    ui->brushRadius->setValue(50);
    ui->brushWeight->setValue(100);

#ifdef USE_SHADER_MATERIALS
    ui->materialCurvature->setChecked(true);
#else
    // Hide the material picker if we don't have shaders
    ui->materials->setVisible(false);
#endif

    setWindowState(windowState() | Qt::WindowMaximized);
    fileNew();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::modeChanged()
{
    if (ui->actionAddJoints->isChecked()) ui->view->setMode(MODE_ADD_JOINTS);
    else if (ui->actionScaleJoints->isChecked()) ui->view->setMode(MODE_SCALE_JOINTS);
    else if (ui->actionEditMesh->isChecked()) ui->view->setMode(MODE_EDIT_MESH);
    else if (ui->actionSculptMesh->isChecked()) ui->view->setMode(MODE_SCULPT_MESH);
    else if (ui->actionAnimate->isChecked()) ui->view->setMode(MODE_ANIMATE);

    ui->brushSettings->setEnabled(ui->actionSculptMesh->isChecked());
    ui->materials->setEnabled(ui->actionSculptMesh->isChecked());
}

void MainWindow::cameraChanged()
{
    ui->view->setCamera(ui->orbitCamera->isChecked() ? CAMERA_ORBIT : CAMERA_FIRST_PERSON);
}

void MainWindow::fileNew()
{
    if (checkCanOverwriteUnsavedChanges())
    {
        Document *doc = new Document;
        connect(doc, SIGNAL(documentChanged()), this, SLOT(documentChanged()));
        documentChanged();
        doc->mesh.balls += Ball(Vector3(), 0.5);
        ui->view->setDocument(doc);
        updateMode();

        filePath = QString::null;
        fileName = "Untitled";
        updateTitle();
    }
}

void MainWindow::fileOpen()
{
    if (!checkCanOverwriteUnsavedChanges())
        return;

    QString path = QFileDialog::getOpenFileName(this, "Open", getDirectory(), DIALOG_FILE_FILTER);
    if (path.isEmpty())
        return;

    Document *doc = new Document;
    if (doc->mesh.loadFromOBJ(path.toStdString()))
    {
        connect(doc, SIGNAL(documentChanged()), this, SLOT(documentChanged()));
        documentChanged();
        setDirectory(QDir(path).absolutePath());
        doc->mesh.uploadToGPU();
        ui->view->setDocument(doc);
        updateMode();
        filePath = path;
        updateTitle();
    }
    else
    {
        delete doc;
        QString text = QString("Could not read from \"%1\"").arg(path);
        QMessageBox::information(this, WINDOW_TITLE, text, QMessageBox::Ok);
    }
}

bool MainWindow::fileSave()
{
    if (filePath.isEmpty())
    {
        // ask the user where to save it if they haven't already said
        return fileSaveAs();
    }
    else if (!ui->view->getDocument().mesh.saveToOBJ(filePath.toStdString()))
    {
        QString text = QString("Could not write to \"%1\"").arg(filePath);
        QMessageBox::information(this, WINDOW_TITLE, text, QMessageBox::Ok);
        return false;
    }

    ui->view->getDocument().getUndoStack().setClean();
    updateTitle();
    return true;
}

bool MainWindow::fileSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save", getDirectory(), DIALOG_FILE_FILTER);
    if (path.isEmpty())
        return false;

    // make sure the file name ends in FILE_EXTENSION
    if (!path.endsWith(FILE_EXTENSION))
        path += FILE_EXTENSION;

    if (!ui->view->getDocument().mesh.saveToOBJ(path.toStdString()))
    {
        // don't set filePath if we can't write to it
        QString text = QString("Could not write to \"%1\"").arg(path);
        QMessageBox::information(this, WINDOW_TITLE, text, QMessageBox::Ok);
        return false;
    }

    ui->view->getDocument().getUndoStack().setClean();
    setDirectory(QDir(path).absolutePath());
    filePath = path;
    updateTitle();
    return true;
}

void MainWindow::fileExit()
{
    if (checkCanOverwriteUnsavedChanges())
        QApplication::exit();
}

void MainWindow::editUndo()
{
    ui->view->undo();
}

void MainWindow::editRedo()
{
    ui->view->redo();
}

void MainWindow::documentChanged()
{
    // This is called in the middle of QUndoStack processing, but we need
    // to handle changes to undo and redo after QUndoStack has finished.
    QApplication::postEvent(this, new QEvent(UPDATE_UNDO_REDO));
}

void MainWindow::runEverything()
{
    Mesh mesh;
    Document &doc = ui->view->getDocument();
    mesh.balls = doc.mesh.balls;
    mesh.updateChildIndices();

    // Run the algorithm
    MeshConstruction::BMeshInit(mesh);
    for (int i = 0; i < 3; i++)
    {
        CatmullMesh::subdivide(mesh);
        MeshEvolution::run(mesh);
        EdgeFairing::run(mesh, 15);
    }

    doc.getUndoStack().beginMacro("Run Everything");
    doc.changeMesh(mesh.vertices, mesh.triangles, mesh.quads);
    doc.getUndoStack().endMacro();
    updateMode();
}

void MainWindow::generateMesh()
{
    Mesh mesh;
    Document &doc = ui->view->getDocument();
    mesh.balls = doc.mesh.balls;
    mesh.updateChildIndices();
    MeshConstruction::BMeshInit(mesh);
    doc.getUndoStack().beginMacro("Generate Mesh");
    doc.changeMesh(mesh.vertices, mesh.triangles, mesh.quads);
    doc.getUndoStack().endMacro();
    updateMode();
}

void MainWindow::subdivideMesh()
{
    Mesh mesh;
    Document &doc = ui->view->getDocument();
    CatmullMesh::subdivide(doc.mesh, mesh);
    doc.getUndoStack().beginMacro("Subdivide Mesh");
    doc.changeMesh(mesh.vertices, mesh.triangles, mesh.quads);
    doc.mesh.subdivisionLevel += 1;
    doc.getUndoStack().endMacro();
    updateMode();
}

void MainWindow::convexHull()
{
    Mesh mesh;
    Document &doc = ui->view->getDocument();
    mesh.vertices = doc.mesh.vertices;
    ConvexHull3D::run(mesh);
    doc.getUndoStack().beginMacro("Convex Hull");
    doc.changeMesh(mesh.vertices, mesh.triangles, mesh.quads);
    doc.getUndoStack().endMacro();
    updateMode();
}

void MainWindow::evolveMesh()
{
    Document &doc = ui->view->getDocument();
    Mesh mesh = doc.mesh;
    MeshEvolution::run(mesh);
    doc.getUndoStack().beginMacro("Evolve Mesh");
    doc.changeMesh(mesh.vertices, mesh.triangles, mesh.quads);
    doc.getUndoStack().endMacro();
    updateMode();
}

void MainWindow::edgeFairing()
{
    Document &doc = ui->view->getDocument();
    Mesh mesh = doc.mesh;
    EdgeFairing::run(mesh, 5);
    doc.getUndoStack().beginMacro("Edge Fairing");
    doc.changeMesh(mesh.vertices, mesh.triangles, mesh.quads);
    doc.getUndoStack().endMacro();
    updateMode();
}

void MainWindow::trianglesToQuads()
{
    Document &doc = ui->view->getDocument();
    Mesh mesh = doc.mesh;
    TrianglesToQuads::run(mesh);
    doc.getUndoStack().beginMacro("Triangles To Quads");
    doc.changeMesh(mesh.vertices, mesh.triangles, mesh.quads);
    doc.getUndoStack().endMacro();
    updateMode();
}

void MainWindow::brushModeChanged()
{
    if (ui->brushAddOrSubtract->isChecked()) ui->view->setBrushMode(BRUSH_ADD_OR_SUBTRACT);
    else if (ui->brushSmooth->isChecked()) ui->view->setBrushMode(BRUSH_SMOOTH);
    else if (ui->brushGrab->isChecked()) ui->view->setBrushMode(BRUSH_GRAB);
}

void MainWindow::brushRadiusChanged(int value)
{
    float brushRadius = (float)value / 100;
    ui->view->setBrushRadius(brushRadius);
    ui->brushRadiusLabel->setText(QString("Radius: %1").arg(brushRadius));
}

void MainWindow::brushWeightChanged(int value)
{
    float brushWeight = (float)value / 100;
    ui->view->setBrushWeight(brushWeight / 4);
    ui->brushWeightLabel->setText(QString("Strength: %1").arg(brushWeight));
}

void MainWindow::materialChanged()
{
    if (ui->materialCurvature->isChecked()) ui->view->setMaterial(MATERIAL_CURVATURE);
    else if (ui->materialMapleCandy->isChecked()) ui->view->setMaterial(MATERIAL_MAPLE_CANDY);
    else if (ui->materialMetal->isChecked()) ui->view->setMaterial(MATERIAL_METAL);
    else if (ui->materialRedWax->isChecked()) ui->view->setMaterial(MATERIAL_RED_WAX);
}

void MainWindow::updateMode()
{
    Mesh &mesh = ui->view->getDocument().mesh;
    if (mesh.balls.isEmpty() || mesh.triangles.count() + mesh.quads.count() > 0)
    {
        // try to keep the currently checked action
        QAction *checked = ui->actionSculptMesh->isChecked() ? ui->actionSculptMesh : ui->actionAnimate->isChecked() ? ui->actionAnimate : ui->actionEditMesh;

        ui->actionAddJoints->setEnabled(false);
        ui->actionScaleJoints->setEnabled(false);
        ui->actionEditMesh->setEnabled(true);
        ui->actionSculptMesh->setEnabled(true);
        ui->actionAnimate->setEnabled(true);

        checked->setChecked(true);
    }
    else
    {
        // try to keep the currently checked action
        QAction *checked = ui->actionScaleJoints->isChecked() ? ui->actionScaleJoints : ui->actionAddJoints;

        ui->actionAddJoints->setEnabled(true);
        ui->actionScaleJoints->setEnabled(true);
        ui->actionEditMesh->setEnabled(false);
        ui->actionSculptMesh->setEnabled(false);
        ui->actionAnimate->setEnabled(false);

        checked->setChecked(true);
    }
    ui->view->updateGL();
}

void MainWindow::updateTitle()
{
    if (!filePath.isEmpty())
    {
        // extract the last path component for the name
        fileName = filePath;
        fileName.remove(0, fileName.lastIndexOf('/') + 1);
    }

    // put an asterisk after the name of unsaved documents
    setWindowTitle(fileName + (ui->view->getDocument().getUndoStack().isClean() ? " - " WINDOW_TITLE : "* - " WINDOW_TITLE));
}

void MainWindow::updateUndoRedo()
{
    // update the text and enabled state of the undo and redo commands
    QUndoStack &undoStack = ui->view->getDocument().getUndoStack();
    ui->actionUndo->setText("Undo " + undoStack.undoText());
    ui->actionRedo->setText("Redo " + undoStack.redoText());
    ui->actionUndo->setEnabled(undoStack.canUndo());
    ui->actionRedo->setEnabled(undoStack.canRedo());
    ui->actionUndoToolbar->setEnabled(undoStack.canUndo());
    ui->actionRedoToolbar->setEnabled(undoStack.canRedo());
}

bool MainWindow::checkCanOverwriteUnsavedChanges()
{
    if (ui->view->getDocument().getUndoStack().isClean())
        return true;

    QString text = QString("Do you want to save changes you made to the document \"%1\"?").arg(fileName);
    int result = QMessageBox::warning(this, WINDOW_TITLE, text, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (result)
    {
    case QMessageBox::Save:
        return fileSave();

    case QMessageBox::Discard:
        return true;
    }

    return false;
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == UPDATE_UNDO_REDO)
    {
        updateUndoRedo();
        updateMode();
        return true;
    }

    return QMainWindow::event(event);
}

void MainWindow::setDirectory(const QString &dir)
{
    QSettings settings(SETTINGS_NAME);
    settings.setValue(SETTING_DIRECTORY, dir);
}

QString MainWindow::getDirectory()
{
    QSettings settings(SETTINGS_NAME);
    return settings.value(SETTING_DIRECTORY, QDir::homePath()).toString();
}
