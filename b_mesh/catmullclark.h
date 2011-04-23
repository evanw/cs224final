#ifndef CATMULLCLARK_H
#define CATMULLCLARK_H

#include <QVector>
#include "mesh.h"

/**
  A library for Catmull-Clark subdivision.
  To subdivide a mesh, call CatmullMesh::subdivide(inputMesh, outputMesh);
  **/

// a face in a CatmullMesh
struct CatmullFace {
    CatmullFace(int numSides);

    int n;
    Vertex facePoint;
    // these are filled to the correct size in the constructor, so they can be treated like arrays after construction
    QVector<const CatmullFace *> neighbors;
    QVector<int> points;
    QVector<int> edgePoints;
};

// a vertex in a CatmullMesh
struct CatmullVertex {
    Vector3 pos;
    Vector3 normal;

    QVector<Vector3> facePoints; // face points for faces including this vertex
    QVector<Vector3> edgePoints; // edge points for faces including this vertex
};


class CatmullMesh {
public:
    // create the mesh Catmull mesh from a regular mesh
    CatmullMesh(const Mesh &m);

    bool convertToMesh(Mesh &m);
    static bool subdivide(const Mesh &in, Mesh &out);

private:
    QList<CatmullVertex> vertices;
    QList<Vertex> edgePoints;
    QList<CatmullFace> faces;

    void fillNeighbors(CatmullFace &face);
    bool moveVertices();
};


#endif // CATMULLCLARK_H
