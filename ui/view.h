#ifndef CANVAS_H
#define CANVAS_H

#include <QGLWidget>
#include "tools.h"
#include "camera.h"
#include "document.h"

enum
{
    MODE_ADD_JOINTS,
    MODE_SCALE_JOINTS,
    MODE_EDIT_MESH
};

class View : public QGLWidget
{
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

    void setMode(int mode);
    void setDocument(Document *doc);
    Document &getDocument() { return *doc; }

    void undo();
    void redo();

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

    bool mirrorChanges;
    bool drawWireframe;
    bool drawInterpolated;
    OrbitCamera camera;

    Tool *currentTool;
    QList<Tool *> tools;
    friend class Tool;
    friend class OrbitCameraTool;
    friend class MoveSelectionTool;
    friend class ScaleSelectionTool;
    friend class SetAndMoveSelectionTool;
    friend class SetAndScaleSelectionTool;
    friend class CreateBallTool;

    void resetTools();
    void resetCamera();
    void resetInteraction();
    void drawMesh() const;
    void drawSkeleton(bool drawTransparent) const;
    void drawGroundPlane() const;
    void camera2D() const;
    void camera3D() const;

public slots:
    void setMirrorChanges(bool useMirrorChanges);
    void setWireframe(bool useWireframe);
    void setInterpolated(bool useInterpolated);
    void deleteSelection();
};

#endif // CANVAS_H
