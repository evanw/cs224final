#ifndef MESHEVOLUTION_H
#define MESHEVOLUTION_H

#include "mesh.h"

/**
 * Defines a signed distance field around a sphere swept from one sphere to another.
 */
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
    bool onlyUseA;

public:
    Sweep() {}
    Sweep(const Ball &ballA, const Ball &ballB) :
            centerA(ballA.center), centerB(ballB.center), AtoB(centerB - centerA),
            radiusA(ballA.maxRadius()), radiusB(ballB.maxRadius()),
            AtoB_lengthSquared(AtoB.lengthSquared()), onlyUseA(false)
    {
        float distance = (centerA - centerB).length();
        if (distance + radiusA <= radiusB)
        {
            // A is inside B
            centerA = centerB;
            radiusA = radiusB;
            onlyUseA = true;
        }
        else if (distance + radiusB <= radiusA)
        {
            // B is inside A
            onlyUseA = true;
        }
    }

    float scalarField(const Vector3 &pos) const;
};

class MeshEvolution
{
private:
    MeshEvolution();

    Mesh &mesh;
    QVector<Sweep> sweeps;

    MeshEvolution(Mesh &mesh);

    float scalarField(const Vector3 &pos) const;
    Vector3 scalarFieldNormal(const Vector3 &pos) const;
    void evolve() const;

public:
    static void run(Mesh &mesh);
    static void drawDebug(Mesh &mesh, float min, float max, int increments);
};

#endif // MESHEVOLUTION_H
