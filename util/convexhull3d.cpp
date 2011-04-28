#include "convexhull3d.h"
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
