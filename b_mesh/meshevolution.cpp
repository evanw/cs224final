#include "meshevolution.h"
#include "curvature.h"
#include <float.h>


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
    }
}
