#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class Document;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void modeChanged();
    void cameraChanged();
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void fileExit();
    void editUndo();
    void editRedo();
    void documentChanged();
    void runEverything();
    void generateMesh();
    void subdivideMesh();
    void convexHull();
    void evolveMesh();
    void edgeFairing();
    void trianglesToQuads();
    void brushModeAddOrSubtract();
    void brushModeSmooth();
    void brushRadiusChanged(int value);
    void brushWeightChanged(int value);

private:
    Ui::MainWindow *ui;
    QString filePath;
    QString fileName;

    void updateMode();
    void updateTitle();
    void updateUndoRedo();
    bool checkCanOverwriteUnsavedChanges();
    bool event(QEvent *event);

    void setDirectory(const QString &dir);
    QString getDirectory();
};

#endif // MAINWINDOW_H
