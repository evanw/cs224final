#include "edgefairing.h"

void EdgeFairing::computeNeighbors()
{
    vertexInfo.resize(mesh.vertices.count());

    foreach (const Triangle &tri, mesh.triangles)
    {
        vertexInfo[tri.a.index].neighbors += tri.b.index;
        vertexInfo[tri.a.index].neighbors += tri.c.index;

        vertexInfo[tri.b.index].neighbors += tri.c.index;
        vertexInfo[tri.b.index].neighbors += tri.a.index;

        vertexInfo[tri.c.index].neighbors += tri.a.index;
        vertexInfo[tri.c.index].neighbors += tri.b.index;
    }

    foreach (const Quad &quad, mesh.quads)
    {
        vertexInfo[quad.a.index].neighbors += quad.b.index;
        vertexInfo[quad.a.index].neighbors += quad.d.index;

        vertexInfo[quad.b.index].neighbors += quad.c.index;
        vertexInfo[quad.b.index].neighbors += quad.a.index;

        vertexInfo[quad.c.index].neighbors += quad.d.index;
        vertexInfo[quad.c.index].neighbors += quad.b.index;

        vertexInfo[quad.d.index].neighbors += quad.a.index;
        vertexInfo[quad.d.index].neighbors += quad.c.index;
    }
}

void EdgeFairing::iterate()
{
    for (int i = 0; i < vertexInfo.count(); i++)
    {
        Vertex &vertex = mesh.vertices[i];
        VertexInfo &info = vertexInfo[i];

        if (info.neighbors.count() == 4)
        {
            // TODO: special-case valence == 4
            info.nextPos = vertex.pos;
        }
        else
        {
            // calculate the average neighbor vertex position
            Vector3 average;
            foreach (int neighbor, info.neighbors)
                average += mesh.vertices[neighbor].pos;
            average /= info.neighbors.count();

            // move the vertex to the average projected onto the tangent plane
            float t = (vertex.pos - average).dot(vertex.normal);
            info.nextPos = average + vertex.normal * t;
        }
    }

    // actually move the vertices (must be done in a separate loop or we would be mutating while iterating)
    for (int i = 0; i < vertexInfo.count(); i++)
        mesh.vertices[i].pos = vertexInfo[i].nextPos;

    mesh.updateNormals();
}

void EdgeFairing::run(Mesh &mesh)
{
    EdgeFairing edgeFairing(mesh);
    edgeFairing.computeNeighbors();
    edgeFairing.iterate();
}
