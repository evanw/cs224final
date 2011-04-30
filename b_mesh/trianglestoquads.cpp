#include "trianglestoquads.h"
#include <QHash>

inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }

Vector3 getNormal(const Mesh &mesh, const Triangle &tri)
{
    const Vector3 &a = mesh.vertices[tri.a.index].pos;
    const Vector3 &b = mesh.vertices[tri.b.index].pos;
    const Vector3 &c = mesh.vertices[tri.c.index].pos;
    return (c - a).cross(b - a).unit();
}

void edgeHelper(QVector<int> &indices, int a, int b, const Triangle &tri)
{
    if (tri.a.index == a && tri.b.index == b) indices += tri.c.index;
    else if (tri.b.index == a && tri.c.index == b) indices += tri.a.index;
    else if (tri.c.index == a && tri.a.index == b) indices += tri.b.index;
}

struct Edge
{
    Quad quad;
    int indexA, indexB;
    float score;

    Edge() : indexA(-1), indexB(-1), score(0) {}

    bool computeScore(const Mesh &mesh)
    {
        if (indexA == -1 || indexB == -1) return false;

        const Triangle &triA = mesh.triangles[indexA];
        const Triangle &triB = mesh.triangles[indexB];

        // merge the two triangles
        QVector<int> indices;
        indices += triA.a.index;
        edgeHelper(indices, triA.b.index, triA.a.index, triB);
        indices += triA.b.index;
        edgeHelper(indices, triA.c.index, triA.b.index, triB);
        indices += triA.c.index;
        edgeHelper(indices, triA.a.index, triA.c.index, triB);
        if (indices.count() != 4) return false; // this happens sometimes, like in buddha.obj
        quad = Quad(indices[0], indices[1], indices[2], indices[3]);

        // compute a score
        Vector3 normalA = getNormal(mesh, triA);
        Vector3 normalB = getNormal(mesh, triB);
        const Vector3 &a = mesh.vertices[quad.a.index].pos;
        const Vector3 &b = mesh.vertices[quad.b.index].pos;
        const Vector3 &c = mesh.vertices[quad.c.index].pos;
        const Vector3 &d = mesh.vertices[quad.d.index].pos;
        float ac = (a - c).length();
        float bd = (b - d).length();

        // measure how similar the surface normals are
        score = normalA.dot(normalB);

        // penalize for quads with different length diagonals
        score -= fabsf(ac - bd) / (ac + bd);

        // penalize for quads with different diagonal centers
        score -= (a + c - b - d).length() / (ac + bd);

        return true;
    }

    bool operator < (const Edge &other) const
    {
        return score > other.score;
    }
};

typedef QPair<int, int> Pair;

Pair makePair(int a, int b)
{
    return Pair(min(a, b), max(a, b));
}

void TrianglesToQuads::run(Mesh &mesh)
{
    QHash<Pair, Edge> edges;
    QHash<int, bool> triangleDeleted;

    for (int i = 0; i < mesh.triangles.count(); i++)
    {
        const Triangle &tri = mesh.triangles[i];
        Pair pair;

        pair = makePair(tri.a.index, tri.b.index);
        if (edges.contains(pair)) edges[pair].indexB = i;
        else edges[pair].indexA = i;

        pair = makePair(tri.b.index, tri.c.index);
        if (edges.contains(pair)) edges[pair].indexB = i;
        else edges[pair].indexA = i;

        pair = makePair(tri.c.index, tri.a.index);
        if (edges.contains(pair)) edges[pair].indexB = i;
        else edges[pair].indexA = i;
    }

    // sort triangles by score
    QVector<Edge> sortedEdges;
    QHashIterator<Pair, Edge> i(edges);
    while (i.hasNext())
    {
        i.next();
        Edge &edge = edges[i.key()];
        if (edge.computeScore(mesh))
            sortedEdges += edge;
    }
    qSort(sortedEdges);

    // convert triangles to quads
    foreach (const Edge &edge, sortedEdges)
    {
        if (triangleDeleted[edge.indexA] || triangleDeleted[edge.indexB])
            continue;

        triangleDeleted[edge.indexA] = true;
        triangleDeleted[edge.indexB] = true;
        mesh.quads += edge.quad;
    }

    // delete triangles
    QVector<Triangle> triangles;
    for (int i = 0; i < mesh.triangles.count(); i++)
        if (!triangleDeleted[i])
            triangles += mesh.triangles[i];
    mesh.triangles = triangles;
}
