#ifndef MESHEVOLUTION_H
#define MESHEVOLUTION_H

#include "mesh.h"

class MeshEvolution
{
private:
    MeshEvolution();

    Mesh &mesh;
    QVector<Ball> balls;
    QVector<float> maxCurvatures;
    QVector<float> minCurvatures;
    float step;

    MeshEvolution(Mesh &mesh);

    float scalarField(const Vector3 &pos) const;
    Vector3 scalarFieldNormal(const Vector3 &pos) const;
    void evolve(float time) const;
    float getMaxTimestep() const;
    float motionSpeed(int vertIndex) const; // motion speed function F
    void testEvolve(float time) const;

public:
    static void run(Mesh &mesh);
    static void drawDebug(Mesh &mesh, float min, float max, int increments);
};

#endif // MESHEVOLUTION_H
