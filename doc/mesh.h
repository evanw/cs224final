#ifndef MESH_H
#define MESH_H

#include <QVector>
#include <string>
#include <QQuaternion>
#include "vector.h"

#define BALL_DETAIL 16

// 0 = use glBegin() and glEnd() blocks, use for older platforms or if vertex buffers don't work
// 1 = use vertex buffers, will be faster because data stays on the GPU between frames
#define ENABLE_GPU_UPLOAD 0

struct Ball
{
    Vector3 center;

    // local coordinate frame of ellipsoid (includes scale factors)
    Vector3 ex, ey, ez;

    // rotation quaternion and translation vector
    QQuaternion rotation;
    Vector3 translation;

    // index into Mesh::balls
    int parentIndex;

    // temporary values used by b_mesh code, are only set immediately before calling
    QVector<int> childrenIndices;

    Ball() : ex(1, 0, 0), ey(0, 1, 0), ez(0, 0, 1), parentIndex(-1) {}
    Ball(const Vector3 &center, float radius, int parentIndex = -1) : center(center), ex(radius, 0, 0), ey(0, radius, 0), ez(0, 0, radius), parentIndex(parentIndex) {}

    float minRadius() const { return min(min(ex.length(), ey.length()), ez.length()); }
    float maxRadius() const { return max(max(ex.length(), ey.length()), ez.length()); }
    bool isOppositeOf(const Ball &other) const;
    void draw(int detail) const;
};

struct Vertex
{
    Vector3 pos;
    Vector3 normal;
    float jointWeights[2];
    int jointIndices[2];

    Vertex() {
        jointWeights[0] = jointWeights[1] = 0;
        jointIndices[0] = jointIndices[1] = -1;
    }
    Vertex(const Vector3 &pos) : pos(pos) {
        jointWeights[0] = jointWeights[1] = 0;
        jointIndices[0] = jointIndices[1] = -1;
    }
    Vertex(const Vector3 &pos, int jointIndex) : pos(pos) {
        jointWeights[0] = 1;
        jointWeights[1] = 0;
        jointIndices[0] = jointIndex;
        jointIndices[1] = -1;
    }

    void draw() const;
};

struct Index
{
    // index into Mesh::vertices
    int index;

    // texture coordinate
    Vector2 coord;

    Index() : index(0) {}
    Index(int index) : index(index) {}
};

struct Triangle
{
    Index a, b, c;

    Triangle() {}
    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}
};

struct Quad
{
    Index a, b, c, d;

    Quad() {}
    Quad(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}
};

class Mesh
{
private:
#if ENABLE_GPU_UPLOAD
    unsigned int vertexBuffer;
    unsigned int triangleIndexBuffer;
    unsigned int lineIndexBuffer;
#endif
    QVector<Vertex> cachedVertices;
    QVector<int> cachedTriangleIndices;
    QVector<int> cachedLineIndices;

public:
    QVector<Ball> balls;
    QVector<Vertex> vertices;
    QVector<Triangle> triangles;
    QVector<Quad> quads;
    int subdivisionLevel;

    Mesh();
    ~Mesh();

    void updateChildIndices();
    void updateNormals();
    void uploadToGPU();

    int getOppositeBall(int index) const;
    void drawPoints() const;
    void drawFill() const;
    void drawWireframe() const;
    void drawKeyBalls(float alpha = 1) const;
    void drawInBetweenBalls() const;
    void drawBones() const;

    bool loadFromOBJ(const std::string &file);
    bool saveToOBJ(const std::string &file);

    static const Vector3 symmetryFlip;
};

#endif // MESH_H
