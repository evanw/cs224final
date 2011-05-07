#ifndef MESHSCULPTER_H
#define MESHSCULPTER_H

#include "tools.h"
#include "meshacceleration.h"

enum { MESH_GRAB };

class MeshInfo
{
private:
    bool isInitialized;
    Ball *ballPointer;
    Vertex *vertexPointer;
    Triangle *trianglePointer;
    Quad *quadPointer;
    int ballCount;
    int vertexCount;
    int triangleCount;
    int quadCount;

public:
    MeshInfo() : isInitialized(false) {}
    MeshInfo(Mesh &mesh);

    bool operator == (const MeshInfo &other) const;
    bool operator != (const MeshInfo &other) const { return !(*this == other); }
};

class MeshSculpterTool : public Tool
{
private:
    MetaMesh *mesh;
    AccelerationDataStructure *accel;

    // Remember info about the mesh so we can tell when it has changed
    MeshInfo meshInfo;

    void updateAccel();
    void stampBrush(const Vector3 &brushCenter, const Vector3 &brushNormal);

public:
    float brushRadius;

    MeshSculpterTool(View *view);
    ~MeshSculpterTool();

    void drawDebug(int x, int y);
    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

#endif // MESHSCULPTER_H
