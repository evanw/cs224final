#ifndef METAMESH_H
#define METAMESH_H

#include "mesh.h"

class MetaVertex
{
public:
    bool marked;
    Vector3 &pos;
    Vector3 &normal;
    Vector3 prevPos;
    QVector<Quad *> neighbors;

    MetaVertex(Vertex &vertex) : marked(false), pos(vertex.pos), normal(vertex.normal), prevPos(vertex.pos) {}
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
