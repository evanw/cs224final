#include "meshevolution.h"
#include "curvature.h"
#include <float.h>


class Sweep
{
private:
    float project(const Vector3 &pos)
    {
        return AtoB.dot(pos - centerA) / AtoB_lengthSquared;
    }

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

    float scalarField(const Vector3 &pos)
    {
        Vector3 closest = centerA + AtoB * project(pos);
        Vector3 tilted = closest + AtoB * ((radiusB - radiusA) * (closest - pos).length() / AtoB_lengthSquared);
        float t = max(0, min(1, project(tilted)));
        return (centerA + AtoB * t - pos).length() - (radiusA + (radiusB - radiusA) * t);
    }
};


inline float squared(float x)
{
    return x * x;
}


static float scalarField(const Ball &ball, const Vector3 &pos)
{
    const float alpha = 1.5;
    Vector3 delta = pos - ball.center;
    float ratioSquared =
            squared(delta.dot(ball.ex) / ball.ex.lengthSquared()) +
            squared(delta.dot(ball.ey) / ball.ey.lengthSquared()) +
            squared(delta.dot(ball.ez) / ball.ez.lengthSquared());
    return squared(1 - ratioSquared / squared(alpha));
}


MeshEvolution::MeshEvolution(Mesh &mesh) : mesh(mesh)
{
    // generate in-between balls using the same algorithm as Mesh::drawInBetweenBalls()
    foreach (const Ball &ball, mesh.balls)
    {
        balls += ball;

        // get the parent ball
        if (ball.parentIndex == -1) continue;
        const Ball &parent = balls[ball.parentIndex];

        // decide how many in-between balls to generate
        float totalRadius = ball.maxRadius() + parent.maxRadius();
        float edgeLength = (ball.center - parent.center).length();
        const int count = min(100, ceilf(edgeLength / totalRadius * 4));

        // generate in-between balls
        for (int i = 1; i < count; i++)
        {
            float percent = (float)i / (float)count;
            Ball tween;
            tween.center = Vector3::lerp(ball.center, parent.center, percent);
            tween.ex = Vector3::lerp(ball.ex, parent.ex, percent);
            tween.ey = Vector3::lerp(ball.ey, parent.ey, percent);
            tween.ez = Vector3::lerp(ball.ez, parent.ez, percent);
            balls += tween;
        }
    }

    // get step size
    float minRadius = FLT_MAX;
    foreach (const Ball &ball, mesh.balls) {
        float currRadius = ball.maxRadius();
        if (currRadius < minRadius) {
            minRadius = currRadius;
        }
    }
    // step = min(r_i) / 2^k, k = subdivision level
    step = minRadius / pow(2, mesh.subdivisionLevel);
}

float MeshEvolution::motionSpeed(int vertIndex) const
{
    float fk = 1.f / (1 + fabsf(maxCurvatures[vertIndex]) + fabsf(minCurvatures[vertIndex]));
    // TODO: what's Itarget?
    float Itarget = 0;
    return (scalarField(mesh.vertices[vertIndex].pos) - Itarget) * fk;
}

float MeshEvolution::scalarField(const Vector3 &pos) const
{
    // TODO: threshold parameter
    const float threshold = 0.1f;
    float total = 0;
    foreach (const Ball &ball, balls)
        total += ::scalarField(ball, pos);
    return total - threshold;
}

void MeshEvolution::evolve(float time) const
{
    for (int i = 0; i < mesh.vertices.count(); ++i) {
        Vertex &vertex = mesh.vertices[i];
        vertex.pos += vertex.normal * (motionSpeed(i) * time);
    }
    mesh.updateNormals();
}

float MeshEvolution::getMaxTimestep() const
{
    float fMax = FLT_MIN;

    for (int i = 0, n = mesh.vertices.size(); i < n; ++i) {
        float fCurr = motionSpeed(i);
        if (fCurr > fMax) {
            fMax = fCurr;
        }
    }

    return step / fMax;
}

// simplified evolution based on signed distance
void MeshEvolution::testEvolve(float time) const
{
    for (int i = 0; i < mesh.vertices.size(); ++i) {
        Vertex &v = mesh.vertices[i];
        Vector3 minDiff;
        const Ball *minBall;
        float minLengthSquared = FLT_MAX;
        // find closest point
        foreach (const Ball &b, balls) {
            Vector3 diff = v.pos - b.center;
            float lengthSquared = diff.lengthSquared();
            if (lengthSquared < minLengthSquared) {
                minLengthSquared = lengthSquared;
                minDiff = diff;
                minBall = &b;
            }
        }

        Vector3 newPos = minBall->center + minDiff * (minBall->maxRadius() / sqrtf(minLengthSquared));
        //v.pos = 0.9f * v.pos + 0.1f * newPos;
        v.pos = newPos;
    }
    mesh.updateNormals();
}

void MeshEvolution::run(Mesh &mesh)
{
    MeshEvolution evolution(mesh);
    Curvature curvature;
    curvature.computeCurvatures(mesh);
    evolution.maxCurvatures = curvature.getMaxCurvatures();
    evolution.minCurvatures = curvature.getMinCurvatures();

    // TODO: stop evolution based on error threshold
    for (int i = 0; i < 1; ++i) {
        evolution.evolve(evolution.getMaxTimestep());
        //evolution.testEvolve(0.1f);
    }
}
