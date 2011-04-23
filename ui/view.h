#ifndef CANVAS_H
#define CANVAS_H

#include <QGLWidget>
#include "tools.h"
#include "camera.h"
#include "document.h"

class View : public QGLWidget
{
    Q_OBJECT

public:
    View(QWidget *parent);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    Document doc;
    int selectedBall;

    OrbitCamera camera;

    Tool *currentTool;
    QList<Tool *> tools;

    void drawGrid() const;
    void camera2D() const;
    void camera3D() const;

public slots:
    void undo();
    void redo();
};

#endif // CANVAS_H
