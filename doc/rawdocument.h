#ifndef RAWDOCUMENT_H
#define RAWDOCUMENT_H

#include <QVector>
#include "vector.h"

struct Ellipsoid
{
    Vector3 center;

    // local coordinate frame of ellipsoid (includes scale factors)
    Vector3 ex, ey, ez;

    // index into Mesh::ellipsoids
    int parentIndex;

    Ellipsoid() : ex(1, 0, 0), ey(0, 1, 0), ez(0, 0, 1), parentIndex(-1) {}
};

struct Vertex
{
    Vector3 pos;
};

struct Index
{
    // index into Mesh::vertices
    int index;

    // texture coordinate
    Vector2 coord;
};

struct Triangle
{
    Index a, b, c;
};

struct Quad
{
    Index a, b, c, d;
};

class RawDocument
{
public:
    QVector<Ellipsoid> ellipsoids;
    QVector<Vertex> vertices;
    QVector<Triangle> triangles;
    QVector<Quad> quads;

    void drawKeyBalls() const;
};

#endif // RAWDOCUMENT_H
