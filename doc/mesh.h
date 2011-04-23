#ifndef MESH_H
#define MESH_H

#include <QList>
#include <string>
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
    Vector3 normal;

    Vertex(const Vector3 &pos) : pos(pos) {}

    void draw() const;
};

struct Index
{
    // index into Mesh::vertices
    int index;

    // texture coordinate
    Vector2 coord;

    Index(int index) : index(index) {}
};

struct Triangle
{
    Index a, b, c;

    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}
};

struct Quad
{
    Index a, b, c, d;

    Quad(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}
};

enum { BONE_TYPE_INTERPOLATE, BONE_TYPE_CYLINDER };

class Mesh
{
public:
    QList<Ball> balls;
    QList<Vertex> vertices;
    QList<Triangle> triangles;
    QList<Quad> quads;

    void updateNormals();
    void drawKeyBalls(int boneType) const;
    void drawFill() const;
    void drawWireframe() const;

    bool loadFromOBJ(const std::string &file);
    bool saveToOBJ(const std::string &file);
};

#endif // MESH_H
