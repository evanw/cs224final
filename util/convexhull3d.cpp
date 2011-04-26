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
