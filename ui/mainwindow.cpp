#include "mainwindow.h"
#include "document.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

#define WINDOW_TITLE "cs224final"
#define DIALOG_FILE_FILTER "*.obj"
#define FILE_EXTENSION ".obj"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->showMesh->setChecked(true);
    setWindowState(windowState() | Qt::WindowMaximized);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMode()
{
    if (ui->editSkeleton->isChecked())
        ui->view->setMode(MODE_SKELETON);
    else if (ui->showMesh->isChecked())
        ui->view->setMode(MODE_MESH);
}

void MainWindow::fileNew()
{
    if (checkCanOverwriteUnsavedChanges())
    {
        ui->view->setDocument(new Document);
        filePath = QString::null;
        fileName = "Untitled";
        updateTitle();
    }
}

void MainWindow::fileOpen()
{
    if (!checkCanOverwriteUnsavedChanges())
        return;

    QString path = QFileDialog::getOpenFileName(this, "Open", QDir::homePath(), DIALOG_FILE_FILTER);
    if (path.isEmpty())
        return;

    Document *doc = new Document;
    if (doc->mesh.loadFromOBJ(path.toStdString()))
    {
        ui->view->setDocument(doc);
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

    updateTitle();
    return true;
}

bool MainWindow::fileSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save", QDir::homePath(), DIALOG_FILE_FILTER);
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
    ui->view->getDocument().getUndoStack().undo();
    ui->view->updateGL();
}

void MainWindow::editRedo()
{
    ui->view->getDocument().getUndoStack().redo();
    ui->view->updateGL();
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
