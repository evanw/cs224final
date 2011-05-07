#ifndef MESHEVOLUTION_H
#define MESHEVOLUTION_H

#include "mesh.h"

class Sweep
{
private:
    float project(const Vector3 &pos) const;

    Vector3 centerA;
    Vector3 centerB;
    Vector3 AtoB;
    float radiusA;
    float radiusB;
    float AtoB_lengthSquared;

public:
    Sweep(const Ball &ballA, const Ball &ballB) :
            centerA(ballA.center), centerB(ballB.center), AtoB(centerB - centerA),
            radiusA(ballA.maxRadius()), radiusB(ballB.maxRadius()), AtoB_lengthSquared(AtoB.lengthSquared())
    {
    }

    float scalarField(const Vector3 &pos) const;
};

class MeshEvolution
{
private:
    MeshEvolution();

    Mesh &mesh;
    QVector<Sweep> sweeps;
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
