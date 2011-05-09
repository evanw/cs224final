#ifndef JOINTROTATION_H
#define JOINTROTATION_H

#include "tools.h"


/**
 Tool to rotate joints in a mesh
  */
class JointRotationTool : public Tool
{
    Q_OBJECT

private:
    void updateVertices();
    void calculateAbsoluteRotations();

    // copy of the base mesh from when this tool was instatiated
    Mesh *baseMesh;
    QQuaternion originalRotation;

public:
    JointRotationTool(View *view);
    ~JointRotationTool();

    void drawDebug(int x, int y);
    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

#endif // JOINTROTATION_H
