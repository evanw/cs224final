#ifndef MESHSCULPTER_H
#define MESHSCULPTER_H

#include "tools.h"
#include "meshacceleration.h"

enum { MESH_GRAB };

class MeshSculpterTool : public Tool
{
private:
    MetaMesh mesh;
    AccelerationDataStructure *accel;

    bool hitTestMesh(const Vector3 &origin, const Vector3 &ray, HitTest &result);

public:
    float brushRadius;

    MeshSculpterTool(View *view);
    ~MeshSculpterTool();

    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

#endif // MESHSCULPTER_H
