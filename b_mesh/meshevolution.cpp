#include "meshevolution.h"

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
}

float MeshEvolution::scalarField(const Vector3 &pos) const
{
    // TODO: threshold parameter
    const float threshold = 0;
    float total = 0;
    foreach (const Ball &ball, balls)
        total += ::scalarField(ball, pos);
    return total - threshold;
}

void MeshEvolution::evolve(float time) const
{
    const float target = 0;
    for (int i = 0; i < mesh.vertices.count(); i++)
    {
        Vertex &vertex = mesh.vertices[i];
        float k1 = 0, k2 = 0; // TODO: principal curvatures
        float speed = (scalarField(vertex.pos) - target) / (1 + fabsf(k1) + fabsf(k2));
        vertex.pos += vertex.normal * (speed * time);
    }
    mesh.updateNormals();
}

void MeshEvolution::run(Mesh &mesh)
{
    MeshEvolution evolution(mesh);

    // TODO: adaptive step size
    for (int i = 0; i < 100; i++)
        evolution.evolve(0.01);
}
