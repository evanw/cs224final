#ifndef MESHEVOLUTION_H
#define MESHEVOLUTION_H

#include "mesh.h"

class MeshEvolution
{
private:
    Mesh &mesh;
    QVector<Ball> balls;

    MeshEvolution(Mesh &mesh);

    float scalarField(const Vector3 &pos) const;
    void evolve(float time) const;

public:
    static void run(Mesh &mesh);
};

#endif // MESHEVOLUTION_H
