#ifndef JOINTROTATION_H
#define JOINTROTATION_H

#include "tools.h"
#include "meshinfo.h"
#include <QQuaternion>

/**
 Tool to rotate joints in a mesh
  */
class JointRotationTool : public Tool
{
private:
    QList<int> findRoots();
    void updateBaseMesh();
    void updateVertices();
    void calculateRelativePositions();
    void calculateAbsoluteTransforms();
    void calcTransform(int index, QMatrix4x4 parentTransform);
    void updateBallCenter(int index);

    // rotation quaternion and translation vector (relative to parent)
    QVector<QQuaternion> rotations;
    QVector<Vector3> translations;
    QVector<QMatrix4x4> absoluteTransforms;

    // Remember info about the mesh so we can tell when it has changed
    MeshInfo meshInfo;

    // copy of the base mesh from when this tool was instatiated
    Mesh *baseMesh;
    QQuaternion originalRotation;

    int oldX, oldY;

public:
    JointRotationTool(View *view);
    ~JointRotationTool();

    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

#endif // JOINTROTATION_H
