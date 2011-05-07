#include "meshevolution.h"
#include "curvature.h"
#include <float.h>
#include <qgl.h>


float Sweep::project(const Vector3 &pos)
{
    return AtoB.dot(pos - centerA) / AtoB_lengthSquared;
}

float Sweep::scalarField(const Vector3 &pos)
{
    if (onlyUseA) return (centerA - pos).length() - radiusA;
    Vector3 closest = centerA + AtoB * project(pos);
    Vector3 tilted = closest + AtoB * ((radiusB - radiusA) * (closest - pos).length() / AtoB_lengthSquared);
    float t = max(0, min(1, project(tilted)));
    return (centerA + AtoB * t - pos).length() - (radiusA + (radiusB - radiusA) * t);
}


inline float squared(float x)
{
    return x * x;
}


static float scalarField(const Ball &ball, const Vector3 &pos)
{
    return (ball.center - pos).length() - ball.maxRadius();
}

Vector3 MeshEvolution::scalarFieldNormal(const Vector3 &pos) const
{
    const float e = 1e-2;
    Vector3 dx(e, 0, 0);
    Vector3 dy(0, e, 0);
    Vector3 dz(0, 0, e);

    Vector3 normal(
            scalarField(pos + dx) - scalarField(pos - dx),
            scalarField(pos + dy) - scalarField(pos - dy),
            scalarField(pos + dz) - scalarField(pos - dz)
    );

    float len = normal.length();
    if (len < 1e-6) {
        return Vector3();
    }

    return normal / len;
}

void MeshEvolution::drawDebug(Mesh &mesh, float gridMin, float gridMax, int divisions)
{
    MeshEvolution evo(mesh);

    float x, y, z;
    float increment = (gridMax - gridMin) / divisions;

    glBegin(GL_LINES);
    glColor3f(0, 0, 1);

    x = gridMin;
    for (int i = 0; i < divisions; ++i) {
        y = gridMin;
        for (int j = 0; j < divisions; ++j) {
            z = gridMin;
            for (int k = 0; k < divisions; ++k) {
                glVertex3f(x, y, z);
                glVertex3fv((Vector3(x, y, z) + evo.scalarFieldNormal(Vector3(x, y, z)) * increment * 0.2f).xyz);
                z += increment;
            }
            y += increment;
        }
        x += increment;
    }

    glEnd();
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

        Sweep sweep(ball, parent);

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
    float fk = 1.f;// / (1 + fabsf(maxCurvatures[vertIndex]) + fabsf(minCurvatures[vertIndex]));
    // TODO: what's Itarget?
    float Itarget = 0;
    return -(scalarField(mesh.vertices[vertIndex].pos) - Itarget) * fk;
}

float MeshEvolution::scalarField(const Vector3 &pos) const
{
    // TODO: threshold parameter
    const float threshold = 0.0f;

    float minPos = FLT_MAX, minNeg = 0;
    foreach (const Ball &ball, balls) {
        float curr = ::scalarField(ball, pos);
        if (curr < 0) {
            minNeg = min(minNeg, curr);
        } else {
            minPos = min(minPos, curr);
        }
    }

    if (minNeg == 0) {
        return minPos;
    }
    return minNeg;
}

void MeshEvolution::evolve(float time) const
{
    for (int i = 0; i < mesh.vertices.count(); ++i) {
        Vertex &vertex = mesh.vertices[i];
        vertex.pos += scalarFieldNormal(vertex.pos) * (motionSpeed(i) * time);
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
    //Curvature curvature;
    //curvature.computeCurvatures(mesh);
    //evolution.maxCurvatures = curvature.getMaxCurvatures();
    //evolution.minCurvatures = curvature.getMinCurvatures();

    // TODO: stop evolution based on error threshold
    for (int i = 0; i < 1; ++i) {
        evolution.evolve(.1f);//evolution.getMaxTimestep());
        //evolution.testEvolve(0.1f);
    }
}
