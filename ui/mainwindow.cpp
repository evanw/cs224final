#include "mainwindow.h"
#include "ui_mainwindow.h"

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
        ui->view->setDrawMode(DRAW_MODE_SKELETON);
    else if (ui->showMesh->isChecked())
        ui->view->setDrawMode(DRAW_MODE_MESH);
}
