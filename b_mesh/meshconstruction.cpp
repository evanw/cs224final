#include "meshconstruction.h"
#include "convexhull3d.h"
#include <QHash>

unsigned int qHash(const Vector3 &vec)
{
    unsigned int h1 = qHash(*(int *)&vec.x);
    unsigned int h2 = qHash(*(int *)&vec.y);
    unsigned int h3 = qHash(*(int *)&vec.z);
    unsigned int h12 = ((h1 << 16) | (h1 >> 16)) ^ h2;
    return ((h12 << 16) | (h12 >> 16)) ^ h3;
}

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

static void makeStartOfSweep(Mesh &mesh, const Ball &ball, Vector3 &v0, Vector3 &v1, Vector3 &v2, Vector3 &v3)
{
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
    v0 = ball.center + y + z;
    v1 = ball.center - y + z;
    v2 = ball.center - y - z;
    v3 = ball.center + y - z;
}

static void makeCap(Mesh &mesh, const Ball &ball, ResultQuad &result)
{
    // if we have no parent (and no children, since we are in this method), then just make a box
    if (ball.parentIndex == -1)
    {
        float r = ball.maxRadius();
        Vector3 x,y,z;
        x = X*r;
        y = Y*r;
        z = Z*r;

        Vector3 v0, v1, v2, v3, v4, v5, v6, v7;
        v0 = ball.center;
        v1 = ball.center;
        v2 = ball.center;
        v3 = ball.center;
        v4 = ball.center;
        v5 = ball.center;
        v6 = ball.center;
        v7 = ball.center;

        v0 += y + x;
        v0 += z;
        v1 -= y - x;
        v1 += z;
        v2 -= y - x;
        v2 -= z;
        v3 += y + x;
        v3 -= z;
        v4 += y - x;
        v4 += z;
        v5 -= y + x;
        v5 += z;
        v6 -= y + x;
        v6 -= z;
        v7 += y - x;
        v7 -= z;

        int i = mesh.vertices.count();
        mesh.vertices += Vertex(v0);
        mesh.vertices += Vertex(v1);
        mesh.vertices += Vertex(v2);
        mesh.vertices += Vertex(v3);
        mesh.vertices += Vertex(v4);
        mesh.vertices += Vertex(v5);
        mesh.vertices += Vertex(v6);
        mesh.vertices += Vertex(v7);

        mesh.quads += Quad(i,i+1,i+2,i+3);
        mesh.quads += Quad(i+7,i+6,i+5,i+4);
        mesh.quads += Quad(i+7,i+4,i,i+3);
        mesh.quads += Quad(i+6,i+7,i+3,i+2);
        mesh.quads += Quad(i+6,i+2,i+1,i+5);
        mesh.quads += Quad(i,i+4,i+5,i+1);

        return;
    }

    //these are the edge of the sphere
    Ball parent = mesh.balls.at(ball.parentIndex);
    Vector3 x = -(parent.center - ball.center).unit() * ball.maxRadius();
    Vector3 v0, v1, v2, v3, v4, v5, v6, v7;
    makeStartOfSweep(mesh, ball, v4, v5, v6, v7);
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

static void addSegmentedSweep(Mesh &mesh,
                              ResultQuad &startQuad,
                              const Vector3 &end0, const Vector3 &end1, const Vector3 &end2, const Vector3 &end3,
                              float startRadius, float endRadius)
{
    Vector3 start = (mesh.vertices[startQuad.i0].pos + mesh.vertices[startQuad.i1].pos + mesh.vertices[startQuad.i2].pos + mesh.vertices[startQuad.i3].pos) / 4;
    Vector3 end = (end0 + end1 + end2 + end3) / 4;
    float startToEnd = (end - start).length();
    const int divisions = max(0, (startToEnd - startRadius - endRadius) / (startRadius + endRadius));
    int i0 = startQuad.i0;
    int i1 = startQuad.i1;
    int i2 = startQuad.i2;
    int i3 = startQuad.i3;

    for (int i = 0; i <= divisions; i++)
    {
        if (i == divisions)
        {
            // special-case the end, which is at a different angle
            mesh.vertices += end0;
            mesh.vertices += end1;
            mesh.vertices += end2;
            mesh.vertices += end3;
        }
        else
        {
            // calculate how far along the bone we are, starting from after startRadius and ending before endRadius
            // this will go negative if the spheres are intersecting, but in that case divisions == 0 so it doesn't matter
            float percent = (divisions > 1) ? (float)i / (float)(divisions - 1) : 0;
            float scale = (startRadius + (startToEnd - startRadius - endRadius) * percent) / startToEnd;
            Vector3 offset = start + (end - start) * scale;

            // interpolate the position along the bone, growing or shrinking based on startRadius, endRadius, and how far along the bone we are
            scale = (startRadius + (endRadius - startRadius) * scale) / startRadius;
            mesh.vertices += offset + (startQuad.v[0] - start) * scale;
            mesh.vertices += offset + (startQuad.v[1] - start) * scale;
            mesh.vertices += offset + (startQuad.v[2] - start) * scale;
            mesh.vertices += offset + (startQuad.v[3] - start) * scale;
        }

        // generate the quads
        int i = mesh.vertices.count() - 4;
        mesh.quads += Quad(i, i + 1, i1, i0);
        mesh.quads += Quad(i + 1, i + 2, i2, i1);
        mesh.quads += Quad(i + 2, i + 3, i3, i2);
        mesh.quads += Quad(i + 3, i, i0, i3);

        i0 = i;
        i1 = i + 1;
        i2 = i + 2;
        i3 = i + 3;
    }
}

static void makeElbow(Mesh &mesh, const Ball &ball, ResultQuad &result)
{
    Ball &child = mesh.balls[ball.childrenIndices[0]];
    ResultQuad last;
    sweep(mesh, child, last);

    if (ball.parentIndex == -1)
    {
        // we are the root node and we have one child, so cap off other end
        float scale = ball.maxRadius() / child.maxRadius();
        Vector3 v[4];
        for (int j = 0; j < 4; j++) {
            v[j] = ball.center + (last.v[j] - child.center) * scale;
        }
        addSegmentedSweep(mesh, last, v[0], v[1], v[2], v[3], child.maxRadius(), ball.maxRadius());
        int i = mesh.vertices.count() - 4;
        Vector3 offset = ball.center + (ball.center - child.center).unit() * ball.maxRadius();
        for (int j = 0; j < 4; j++) {
            mesh.vertices += offset + (last.v[j] - child.center) * scale;
        }
        mesh.quads += Quad(i, i + 4, i + 5, i + 1);
        mesh.quads += Quad(i + 1, i + 5, i + 6, i + 2);
        mesh.quads += Quad(i + 2, i + 6, i + 7, i + 3);
        mesh.quads += Quad(i + 3, i + 7, i + 4, i);
        mesh.quads += Quad(i + 4, i + 7, i + 6, i + 5);
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
    addSegmentedSweep(mesh, last, v[0], v[1], v[2], v[3], child.maxRadius(), ball.maxRadius());

    // rotate 100% for the next step
    for (int j = 0; j < 4; j++) {
        v[j] = ball.center + rotate(last.v[j] - child.center, rotationAxis, rotationAngle) * scale;
    }
    int i = mesh.vertices.count() - 4;
    result.setIndices(i, i + 1, i + 2, i + 3);
    result.setVertices(v[0], v[1], v[2], v[3]);
}

static void makeJoint(Mesh &mesh, const Ball &ball, ResultQuad &result)
{
    QVector<Quad> quads;

    for (int k = 0; k < ball.childrenIndices.count(); k++)
    {
        Ball &child = mesh.balls[ball.childrenIndices[k]];

        ResultQuad last;
        sweep(mesh, child, last);

        // move the quad center from child to ball
        float scale = ball.maxRadius() / child.maxRadius();
        Vector3 v[4];
        for (int j = 0; j < 4; j++) {
            v[j] = ball.center + (child.center - ball.center).unit() * ball.maxRadius() + (last.v[j] - child.center) * scale;
        }
        addSegmentedSweep(mesh, last, v[0], v[1], v[2], v[3], child.maxRadius(), ball.maxRadius());

        // remember the last quad for convex hull
        int i = mesh.vertices.count() - 4;
        quads += Quad(i, i + 1, i + 2, i + 3);
    }

    // if there's a parent, we have to do a join using a convex hull
    if (ball.parentIndex != -1)
    {
        // create the quad that will be swept up the parent after this
        int i = mesh.vertices.count();
        Vector3 v0, v1, v2, v3;
        Ball &parent = mesh.balls[ball.parentIndex];
        Vector3 offset = (parent.center - ball.center).unit() * ball.maxRadius();
        makeStartOfSweep(mesh, ball, v0, v1, v2, v3);
        mesh.vertices += v0 + offset;
        mesh.vertices += v1 + offset;
        mesh.vertices += v2 + offset;
        mesh.vertices += v3 + offset;
        result.setIndices(i, i + 1, i + 2, i + 3);
        result.setVertices(v0, v1, v2, v3);
        quads += Quad(i, i + 1, i + 2, i + 3);
    }

    // run the convex hull
    Mesh temp;
    QHash<Vector3, int> indexForVector;
    foreach (const Quad &quad, quads)
    {
        // read the quad vertices
        Vector3 v0 = mesh.vertices[quad.a.index].pos;
        Vector3 v1 = mesh.vertices[quad.b.index].pos;
        Vector3 v2 = mesh.vertices[quad.c.index].pos;
        Vector3 v3 = mesh.vertices[quad.d.index].pos;

        // hack: temporarily shrink the size of the quad so things are much less likely to intersect
        Vector3 center = (v0 + v1 + v2 + v3) / 4;
        const float percent = 0.99;
        v0 += (center - v0) * percent;
        v1 += (center - v1) * percent;
        v2 += (center - v2) * percent;
        v3 += (center - v3) * percent;

        // add the vertices to the input of the convex hul algorithm
        temp.vertices += Vertex(v0);
        temp.vertices += Vertex(v1);
        temp.vertices += Vertex(v2);
        temp.vertices += Vertex(v3);

        // remember what maps where for reconstruction
        indexForVector[v0] = quad.a.index;
        indexForVector[v1] = quad.b.index;
        indexForVector[v2] = quad.c.index;
        indexForVector[v3] = quad.d.index;
    }
    ConvexHull3D::run(temp);
    foreach (const Triangle &tri, temp.triangles)
    {
        Vector3 v0 = temp.vertices[tri.a.index].pos;
        Vector3 v1 = temp.vertices[tri.b.index].pos;
        Vector3 v2 = temp.vertices[tri.c.index].pos;

        // if we generate more vertices, then some of the old vertices were inside the convex hull
        // we can't deal with that so skip these (don't add any triangles to the mesh with new vertices)
        if (!indexForVector.contains(v0) ||
            !indexForVector.contains(v1) ||
            !indexForVector.contains(v2))
        {
            cout << "warning: new vertex generated by convex hull, ignoring triangles with this vertex" << endl;
            continue;
        }

        Triangle tri(indexForVector[v0], indexForVector[v1], indexForVector[v2]);
        bool allOnSameQuad = false;

        // is the triangle all on the same quad?
        foreach (const Quad &quad, quads)
        {
            if (quad.a.index != tri.a.index && quad.b.index != tri.a.index && quad.c.index != tri.a.index && quad.d.index != tri.a.index) continue;
            if (quad.a.index != tri.b.index && quad.b.index != tri.b.index && quad.c.index != tri.b.index && quad.d.index != tri.b.index) continue;
            if (quad.a.index != tri.c.index && quad.b.index != tri.c.index && quad.c.index != tri.c.index && quad.d.index != tri.c.index) continue;
            allOnSameQuad = true;
            break;
        }

        // only add the triangle if it won't be inside the mesh
        if (!allOnSameQuad) mesh.triangles += tri;
    }
}

void sweep(Mesh &mesh, const Ball &ball, ResultQuad &result)
{
    if (ball.childrenIndices.isEmpty())
        makeCap(mesh, ball, result);
    else if (ball.childrenIndices.count() == 1)
        makeElbow(mesh, ball, result);
    else
        makeJoint(mesh, ball, result);
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
