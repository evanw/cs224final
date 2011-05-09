#ifndef JOINTROTATION_H
#define JOINTROTATION_H

#include "tools.h"
#include <QHash>


/**
 Tool to rotate joints in a mesh
  */
class JointRotationTool : public Tool
{
    Q_OBJECT

private:
    void updateVertices();
    void calculateAbsoluteTransforms();
    void calcTransform(Ball *ball, QMatrix4x4 parentTransform);

    // copy of the base mesh from when this tool was instatiated
    Mesh *baseMesh;
    QQuaternion originalRotation;
    QHash<Ball *, QMatrix4x4> absoluteTransforms;

public:
    JointRotationTool(View *view);
    ~JointRotationTool();

    void drawDebug(int x, int y);
    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

#endif // JOINTROTATION_H
