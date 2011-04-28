#include "meshconstruction.h"

struct ResultQuad
{
    Vector3 v[4];
    int i0, i1, i2, i3;

    void setVertices(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &d) { v[0] = a; v[1] = b; v[2] = c; v[3] = d; }
    void setIndices(int a, int b, int c, int d) { i0 = a; i1 = b; i2 = c; i3 = d; }
};

// world vectors
Vector3 X(1,0,0);
Vector3 Y(0,1,0);
Vector3 Z(0,0,1);

void sweep(Mesh &m, const Ball &b, ResultQuad &result);

static Vector3 rotate(const Vector3 &p, const Vector3 &v, float radians)
{
    Vector3 temp = Vector3(p);

    Vector3 axisX = Vector3(1, 1, 1).cross(v).unit();
    Vector3 axisY = v.cross(axisX);

    float x = temp.dot(axisX);
    float y = temp.dot(axisY);
    float z = temp.dot(v);
    float sinA = sin(radians);
    float cosA = cos(radians);

    return axisX * (x * cosA - y * sinA) + axisY * (y * cosA + x * sinA) + v * z;
}

static void makeCap(Mesh &mesh, const Ball &ball, ResultQuad &result)
{
    if (ball.parentIndex == -1)
    {
        // TODO: handle this case
        return;
    }

    //this is an end node. find the local vectors
    Ball parent = mesh.balls.at(ball.parentIndex);
    Vector3 boneDirection = parent.center - ball.center;
    Vector3 x,y,z;
    x = -boneDirection.unit();
    if(x.apequal(Y) || x.apequal(-Y)){
        y = X;
        z = Z;
    }else {
        y = Y.cross(x).unit();
        z = x.cross(y).unit();
    }

    float r = ball.maxRadius();
    x*=r;
    y*=r;
    z*=r;

    //make the quad cap
    Vector3 v0, v1, v2, v3, v4, v5, v6, v7;
    v4 = ball.center;
    v5 = ball.center;
    v6 = ball.center;
    v7 = ball.center;

    //these verts are the center of the sphere
    v4 += y;
    v4 += z;
    v5 -= y;
    v5 += z;
    v6 -= y;
    v6 -= z;
    v7 += y;
    v7 -= z;

    //these are the edge of the sphere
    v0 = v4 + x;
    v1 = v5 + x;
    v2 = v6 + x;
    v3 = v7 + x;

    //add the cap vertices to the mesh, keep track of the start index
    int i = mesh.vertices.count();
    mesh.vertices += Vertex(v0);
    mesh.vertices += Vertex(v1);
    mesh.vertices += Vertex(v2);
    mesh.vertices += Vertex(v3);
    mesh.vertices += Vertex(v4);
    mesh.vertices += Vertex(v5);
    mesh.vertices += Vertex(v6);
    mesh.vertices += Vertex(v7);

    //add the quad to the cap
    mesh.quads += Quad(i, i + 1, i + 2, i + 3);

    //now add the sweep to the center of the sphere
    mesh.quads += Quad(i + 4, i + 5, i + 1, i);
    mesh.quads += Quad(i + 5, i + 6, i + 2, i + 1);
    mesh.quads += Quad(i + 6, i + 7, i + 3, i + 2);
    mesh.quads += Quad(i + 7, i + 4, i, i + 3);

    result.setVertices(v4, v5, v6, v7);
    result.setIndices(i + 4, i + 5, i + 6, i + 7);
}

static void addSweep(Mesh &mesh, int i0, int i1, int i2, int i3, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Vector3 &v3)
{
    int i = mesh.vertices.count();

    mesh.vertices += v0;
    mesh.vertices += v1;
    mesh.vertices += v2;
    mesh.vertices += v3;

    mesh.quads += Quad(i, i + 1, i1, i0);
    mesh.quads += Quad(i + 1, i + 2, i2, i1);
    mesh.quads += Quad(i + 2, i + 3, i3, i2);
    mesh.quads += Quad(i + 3, i, i0, i3);
}

static void makeElbow(Mesh &mesh, const Ball &ball, ResultQuad &result)
{
    Ball &child = mesh.balls[ball.childrenIndices[0]];
    ResultQuad last;
    sweep(mesh, child, last);

    if (ball.parentIndex == -1)
    {
        // TODO: handle this case
        return;
    }

    Ball &parent = mesh.balls[ball.parentIndex];

    // calculate rotation
    Vector3 childDirection = child.center - ball.center;
    Vector3 parentDirection = parent.center - ball.center;
    Vector3 rotationAxis = childDirection.cross(parentDirection).unit();
    float rotationAngle = -acos(-childDirection.dot(parentDirection) / childDirection.length() / parentDirection.length());

    // rotate 50% for elbow
    float scale = ball.maxRadius() / child.maxRadius();
    Vector3 v[4];
    for (int j = 0; j < 4; j++) {
        v[j] = ball.center + rotate(last.v[j] - child.center, rotationAxis, rotationAngle / 2) * scale;
    }
    int i = mesh.vertices.count();
    addSweep(mesh, last.i0, last.i1, last.i2, last.i3, v[0], v[1], v[2], v[3]);

    // rotate 100% for the next step
    for (int j = 0; j < 4; j++) {
        v[j] = ball.center + rotate(last.v[j] - child.center, rotationAxis, rotationAngle) * scale;
    }
    result.setIndices(i, i + 1, i + 2, i + 3);
    result.setVertices(v[0], v[1], v[2], v[3]);
}

static void makeJoint(Mesh &mesh, const Ball &ball, ResultQuad &result)
{
    for (int k = 0; k < ball.childrenIndices.count(); k++) {
        Ball &child = mesh.balls[ball.childrenIndices[k]];

        ResultQuad last;
        sweep(mesh, child, last);

        // move the quad center from child to ball
        float scale = ball.maxRadius() / child.maxRadius();
        Vector3 v[4];
        for (int j = 0; j < 4; j++) {
            v[j] = ball.center + (child.center - ball.center).unit() * ball.maxRadius() + (last.v[j] - child.center) * scale;
        }
        int i = mesh.vertices.count();
        addSweep(mesh, last.i0, last.i1, last.i2, last.i3, v[0], v[1], v[2], v[3]);

        // TODO: stitching step instead
        result.setIndices(i, i + 1, i + 2, i + 3);
        result.setVertices(v[0], v[1], v[2], v[3]);
    }
}

void sweep(Mesh &m, const Ball &b, ResultQuad &result)
{
    switch (b.childrenIndices.count())
    {
    case 0:
        makeCap(m, b, result);
        break;

    case 1:
        makeElbow(m, b, result);
        break;

    default:
        makeJoint(m, b, result);
    }
}

void MeshConstruction::BMeshInit(Mesh &m) {
    ResultQuad result;

    //empty the mesh
    m.vertices.clear();
    m.quads.clear();
    m.triangles.clear();

    //call sweep at each root node
    foreach(const Ball &b, m.balls){
        if(b.parentIndex == -1){
            sweep(m, b, result);
        }
    }

    m.updateNormals();
}
