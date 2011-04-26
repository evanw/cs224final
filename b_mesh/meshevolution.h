#ifndef MESHEVOLUTION_H
#define MESHEVOLUTION_H

#include "mesh.h"

class MeshEvolution
{
private:
    Mesh &mesh;
    QVector<Ball> balls;

public:
    MeshEvolution(Mesh &mesh);

    float scalarField(const Vector3 &pos) const;
    void evolve(float time) const;
};

#endif // MESHEVOLUTION_H
