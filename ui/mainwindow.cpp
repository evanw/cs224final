#include "mainwindow.h"
#include "document.h"
#include "ui_mainwindow.h"
#include "meshconstruction.h"
#include "catmullclark.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#define WINDOW_TITLE "cs224final"
#define DIALOG_FILE_FILTER "*.obj"
#define FILE_EXTENSION ".obj"
#define SETTINGS_NAME "cs224final"
#define SETTING_DIRECTORY "file_dialog_dir"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QActionGroup *group = new QActionGroup(this);
    group->addAction(ui->actionAddJoints);
    group->addAction(ui->actionScaleJoints);
    group->addAction(ui->actionEditMesh);

    ui->drawWireframe->setChecked(true);
    ui->drawInterpolated->setChecked(true);

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
}

void MainWindow::fileNew()
{
    if (checkCanOverwriteUnsavedChanges())
    {
        Document *doc = new Document;
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

void MainWindow::editMenuAboutToShow()
{
    // update the text and enabled state of the undo and redo commands
    QUndoStack &undoStack = ui->view->getDocument().getUndoStack();
    ui->actionUndo->setText("Undo " + undoStack.undoText());
    ui->actionRedo->setText("Redo " + undoStack.redoText());
    ui->actionUndo->setEnabled(undoStack.canUndo());
    ui->actionRedo->setEnabled(undoStack.canRedo());
}

void MainWindow::editMenuAboutToHide()
{
    // we must make sure the keyboard shortcuts work when the menu is hidden
    ui->actionUndo->setEnabled(true);
    ui->actionRedo->setEnabled(true);
}

void MainWindow::generateMesh()
{
    Document &doc = ui->view->getDocument();
    doc.mesh.updateChildIndices();
    MeshConstruction::BMeshInit(&doc.mesh);
    doc.mesh.uploadToGPU();
    ui->view->updateGL();
    updateMode();
}

void MainWindow::subdivideMesh()
{
    Document &doc = ui->view->getDocument();
    doc.mesh.updateChildIndices();
    CatmullMesh::subdivide(doc.mesh, doc.mesh);
    doc.mesh.uploadToGPU();
    ui->view->updateGL();
    updateMode();
}

void MainWindow::updateMode()
{
    Mesh &mesh = ui->view->getDocument().mesh;
    if (mesh.balls.isEmpty() || mesh.triangles.count() + mesh.quads.count() > 0)
    {
        ui->actionAddJoints->setEnabled(false);
        ui->actionScaleJoints->setEnabled(false);
        ui->actionEditMesh->setEnabled(true);
        ui->actionEditMesh->setChecked(true);
    }
    else
    {
        ui->actionAddJoints->setEnabled(true);
        ui->actionScaleJoints->setEnabled(true);
        ui->actionEditMesh->setEnabled(false);
        ui->actionAddJoints->setChecked(true);
    }
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
