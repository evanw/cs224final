#ifndef EDGEFAIRING_H
#define EDGEFAIRING_H

#include "document.h"
#include <QSet>

struct VertexInfo
{
    Vector3 nextPos;
    QSet<int> neighbors;
};

class EdgeFairing
{
private:
    Mesh &mesh;
    QVector<VertexInfo> vertexInfo;

    EdgeFairing(Mesh &mesh) : mesh(mesh) {}
    void computeNeighbors();
    void iterate();

public:
    static void run(Mesh &mesh, int iterations);
};

#endif // EDGEFAIRING_H
