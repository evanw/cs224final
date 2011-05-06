#include "metamesh.h"

MetaMesh::MetaMesh(Mesh &mesh) : mesh(mesh)
{
    for (int i = 0; i < mesh.vertices.count(); i++)
        vertices += new MetaVertex(mesh.vertices[i]);

    for (int i = 0; i < mesh.quads.count(); i++)
    {
        Quad &quad = mesh.quads[i];
        vertices[quad.a.index]->neighbors += &quad;
        vertices[quad.b.index]->neighbors += &quad;
        vertices[quad.c.index]->neighbors += &quad;
        vertices[quad.d.index]->neighbors += &quad;
    }
}

MetaMesh::~MetaMesh()
{
    for (int i = 0; i < vertices.count(); i++)
        delete vertices[i];
}
