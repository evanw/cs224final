#include "edgefairing.h"
#include <QtAlgorithms>

// helper to sort vertices by their rotation in a coordinate system
struct rotationInCoordinateSystem
{
    Vector3 origin, axisX, axisY;

    rotationInCoordinateSystem(const Vector3 &origin, const Vector3 &axisX, const Vector3 &axisY) : origin(origin), axisX(axisX), axisY(axisY) {}

    bool operator () (const Vector3 &a, const Vector3 &b)
    {
        float angleA = atan2f(axisX.dot(a - origin), axisY.dot(a - origin));
        float angleB = atan2f(axisX.dot(b - origin), axisY.dot(b - origin));
        return angleA < angleB;
    }
};

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

        // special-case vertices with valence 4
        if (info.neighbors.count() == 4)
        {
            // project the neighbors onto the tangent plane
            QVector<Vector3> projectedNeighbors;
            foreach (int neighbor, info.neighbors)
            {
                Vector3 &pos = mesh.vertices[neighbor].pos;
                projectedNeighbors += pos + vertex.normal * (vertex.pos - pos).dot(vertex.normal);
            }

            // define a tangent-space coordinate system
            Vector3 axisX = (projectedNeighbors[0] - vertex.pos).unit();
            Vector3 axisY = vertex.normal.cross(axisX);

            // sort the vertices by their rotation in the tangent-space coordinate system
            qSort(projectedNeighbors.begin(), projectedNeighbors.end(), rotationInCoordinateSystem(vertex.pos, axisX, axisY));

            // we now have vectors in clockwise or counter-clockwise order
            //
            //      b
            //      |
            //  a---+---c
            //      |
            //      d
            //
            Vector3 &a = projectedNeighbors[0];
            Vector3 &b = projectedNeighbors[1];
            Vector3 &c = projectedNeighbors[2];
            Vector3 &d = projectedNeighbors[3];

            // set the vertex to the intersection of the lines between the two opposite neighbor pairs
            // blend between the intersection and the average for stability
            Vector3 normal = (c - a).cross(vertex.normal).unit();
            float t = (a - b).dot(normal) / (d - b).dot(normal);
            Vector3 intersection = b + (d - b) * max(0, min(1, t));
            Vector3 average = (a + b + c + d) / 4;
            info.nextPos = (average + intersection) / 2;
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
    // only move a small amount per iteration for stability
    for (int i = 0; i < vertexInfo.count(); i++)
        mesh.vertices[i].pos = Vector3::lerp(mesh.vertices[i].pos, vertexInfo[i].nextPos, 0.1);

    mesh.updateNormals();
}

void EdgeFairing::run(Mesh &mesh, int iterations)
{
    EdgeFairing edgeFairing(mesh);
    edgeFairing.computeNeighbors();

    for (int i = 0; i < iterations; i++)
        edgeFairing.iterate();
}
