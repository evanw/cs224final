#include "convexhull3d.h"

#if 0
#include "chull.h"

void ConvexHull3D::run(Mesh &mesh)
{
    QVector<Vector3> vertices;
    foreach (const Vertex &vertex, mesh.vertices)
        vertices += vertex.pos;

    Chull3D ch(vertices[0].xyz, vertices.count());
    ch.compute();
    ch.export_mesh(mesh);
    mesh.updateNormals();
}

#else
#include "Wm5ConvexHull3.h"
#define COMPILE_TIME_ASSERT(pred) switch(0){case 0:case pred:;}

void ConvexHull3D::run(Mesh &mesh)
{
    // reset faces
    mesh.triangles.clear();
    mesh.quads.clear();
    if (mesh.vertices.isEmpty())
        return;

    // copy vertices to array
    QVector<Vector3> vertices;
    foreach (const Vertex &vertex, mesh.vertices)
        vertices += vertex.pos;

    // call library
    COMPILE_TIME_ASSERT(sizeof(Vector3) == sizeof(Wm5::Vector3f));
    Wm5::ConvexHull3f hull(vertices.count(), (Wm5::Vector3f *)vertices[0].xyz, 0.0001f, false, Wm5::Query::QT_INT64);

    // we can also get 0d, 1d, and 2d output, but we can't use those
    if (hull.GetDimension() == 3)
    {
        // I'm assuming that the vertices stay the same because ConvexHull3f doesn't have a way to get the vertices
        assert(hull.GetNumVertices() == mesh.vertices.count());

        // copy indices back
        int numIndices = hull.GetNumSimplices();
        const int *indices = hull.GetIndices();
        for (int i = 0; i < numIndices; i++)
            mesh.triangles += Triangle(indices[i * 3], indices[i * 3 + 1], indices[i * 3 + 2]);
    }
}
#endif

// The following test crashes on mac but passes on linux. It occurs in both 32-bit and 64-bit builds.
// Problem: Chull3D::edges == NULL
// Stacktrace:
// 0  Chull3D::clean_edges()     line 424
// 1  Chull3D::clean_up()        line 389
// 2  Chull3D::construct_hull()  line 273
// 3  Chull3D::compute()         line 200
void run_test()
{
    int x, y, z;

    Mesh mesh;
    x = -1065206744;
    y = 1109001880;
    z = 1086345854;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065448114;
    y = 1109001880;
    z = 1086234086;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065291584;
    y = 1108995782;
    z = 1086044828;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065097663;
    y = 1108995782;
    z = 1086156596;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);

    x = -1065485632;
    y = 1108992531;
    z = 1086842308;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065198027;
    y = 1108992531;
    z = 1086809428;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065168954;
    y = 1108979463;
    z = 1087005190;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065427485;
    y = 1108979463;
    z = 1087038070;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);

    x = -1066241229;
    y = 1108773830;
    z = 1085635440;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065825105;
    y = 1108773830;
    z = 1085552930;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1065709009;
    y = 1108753946;
    z = 1085699306;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);
    x = -1066125133;
    y = 1108753946;
    z = 1085781816;
    mesh.vertices += Vector3(*(float *)&x, *(float *)&y, *(float *)&z);

    ConvexHull3D::run(mesh);
}

#include <iostream>
using namespace std;

struct test {
    test() {
        cout << "running test" << endl;
        run_test();
        cout << "test passed" << endl;
    }
} test;
