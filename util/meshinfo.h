#ifndef MESHINFO_H
#define MESHINFO_H

#include "mesh.h"

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

    void reset() { isInitialized = false; }

    bool operator == (const MeshInfo &other) const;
    bool operator != (const MeshInfo &other) const { return !(*this == other); }
};

#endif // MESHINFO_H
