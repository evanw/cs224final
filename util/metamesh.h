#ifndef METAMESH_H
#define METAMESH_H

#include "mesh.h"

class MetaVertex
{
public:
    int accelData;
    Vertex &wrappedVertex;
    Vector3 &pos;
    Vector3 &normal;
    Vector3 prevPos;
    Vector3 prevNormal;
    QVector<Quad *> neighbors;

    MetaVertex(Vertex &vertex) : accelData(0), wrappedVertex(vertex), pos(vertex.pos), normal(vertex.normal), prevPos(vertex.pos), prevNormal(vertex.normal) {}
};

class MetaMesh
{
public:
    Mesh &mesh;
    QVector<MetaVertex *> vertices;

    MetaMesh(Mesh &mesh);
    ~MetaMesh();
};

#endif // METAMESH_H
