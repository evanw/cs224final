#ifndef CANVAS_H
#define CANVAS_H

#include <QGLWidget>
#include "tools.h"
#include "camera.h"
#include "document.h"
#include "curvature.h"

enum
{
    MODE_ADD_JOINTS,
    MODE_SCALE_JOINTS,
    MODE_EDIT_MESH,
    MODE_SCULPT_MESH,
};

enum
{
    CAMERA_ORBIT,
    CAMERA_FIRST_PERSON,
};

class View : public QGLWidget
{
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

    void setMode(int mode);
    void setCamera(int camera);
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
    int oppositeSelectedBall; // used by tools
    int mouseX, mouseY; // for highlighting the face of the selection cube
    int mode;

    bool mirrorChanges;
    bool drawWireframe;
    bool drawInterpolated;
    bool drawCurvature;

    Camera *currentCamera;
    OrbitCamera orbitCamera;
    FirstPersonCamera firstPersonCamera;

    Tool *currentTool;
    QList<Tool *> tools;
    friend class Tool;
    friend class OrbitCameraTool;
    friend class FirstPersonCameraTool;
    friend class MoveSelectionTool;
    friend class ScaleSelectionTool;
    friend class SetAndMoveSelectionTool;
    friend class SetAndScaleSelectionTool;
    friend class CreateBallTool;
    friend class MeshSculpterTool;

    void clearTools();
    void updateTools();
    void resetCamera();
    void resetInteraction();
    void drawMesh(bool justMesh) const;
    void drawSkeleton(bool drawTransparent) const;
    void drawGroundPlane() const;
    void drawFullscreenQuad() const;
    void camera2D() const;
    void camera3D() const;

public slots:
    void setMirrorChanges(bool useMirrorChanges);
    void setWireframe(bool useWireframe);
    void setInterpolated(bool useInterpolated);
    void setCurvature(bool useCurvature);
    void deleteSelection();
};

#endif // CANVAS_H
