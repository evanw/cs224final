#ifndef MESH_H
#define MESH_H

#include <QList>
#include "vector.h"

struct Ball
{
    Vector3 center;

    // local coordinate frame of ellipsoid (includes scale factors)
    Vector3 ex, ey, ez;

    // index into Mesh::balls
    int parentIndex;

    Ball() : ex(1, 0, 0), ey(0, 1, 0), ez(0, 0, 1), parentIndex(-1) {}
    Ball(const Vector3 &center, float radius, int parentIndex = -1) : center(center), ex(radius, 0, 0), ey(0, radius, 0), ez(0, 0, radius), parentIndex(parentIndex) {}

    float maxRadius() const { return max(max(ex.length(), ey.length()), ez.length()); }
    void draw() const;
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

enum { BONE_TYPE_INTERPOLATE, BONE_TYPE_CYLINDER };

class Mesh
{
public:
    QList<Ball> balls;
    QList<Vertex> vertices;
    QList<Triangle> triangles;
    QList<Quad> quads;

    void drawKeyBalls(int boneType) const;
};

#endif // MESH_H
