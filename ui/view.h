#ifndef CANVAS_H
#define CANVAS_H

#include <QGLWidget>
#include "tools.h"
#include "camera.h"
#include "document.h"

enum { MODE_MESH, MODE_SKELETON };

class View : public QGLWidget
{
    Q_OBJECT

public:
    View(QWidget *parent);

    void setMode(int mode);
    void setDocument(Document *doc);
    Document &getDocument() { return *doc; }

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    Document *doc;
    int selectedBall;
    int mode;

    OrbitCamera camera;
float mx, my;

    Tool *currentTool;
    QList<Tool *> tools;
    friend class SetSelectionTool;
    friend class MoveSelectionTool;

    void resetCamera();
    void drawMesh() const;
    void drawSkeleton(bool drawTransparent) const;
    void drawGroundPlane() const;
    void camera2D() const;
    void camera3D() const;
};

#endif // CANVAS_H
