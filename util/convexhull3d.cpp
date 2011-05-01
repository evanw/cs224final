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
