#ifndef CANVAS_H
#define CANVAS_H

#include <QGLWidget>
#include "tools.h"
#include "camera.h"
#include "document.h"

enum { DRAW_MODE_MESH, DRAW_MODE_SKELETON };

class View : public QGLWidget
{
    Q_OBJECT

public:
    View(QWidget *parent);

    void setDrawMode(int drawMode);

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
    int drawMode;

    OrbitCamera camera;

    Tool *currentTool;
    QList<Tool *> tools;

    void drawMesh() const;
    void drawSkeleton(bool drawTransparent) const;
    void drawGroundPlane() const;
    void camera2D() const;
    void camera3D() const;

public slots:
    void undo();
    void redo();
};

#endif // CANVAS_H
