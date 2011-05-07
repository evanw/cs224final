#include "meshevolution.h"
#include "curvature.h"
#include <float.h>
#include <qgl.h>

float Sweep::project(const Vector3 &pos) const
{
    return AtoB.dot(pos - centerA) / AtoB_lengthSquared;
}

float Sweep::scalarField(const Vector3 &pos) const
{
    // hax that gets the signed distance to  //
    // a cylinder fitted between two spheres //
    //                                       //
    //              ___---^^^/^^^\           //
    //           /^^\       /     \          //
    //           \__/       \     /          //
    //              ^^^---___\___/           //
    //                                       //
    if (onlyUseA) return (centerA - pos).length() - radiusA;
    Vector3 closest = centerA + AtoB * project(pos);
    Vector3 tilted = closest + AtoB * ((radiusB - radiusA) * (closest - pos).length() / AtoB_lengthSquared);
    float t = max(0, min(1, project(tilted)));
    return (centerA + AtoB * t - pos).length() - (radiusA + (radiusB - radiusA) * t);
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
    // generate in-between balls using hax
    foreach (const Ball &ball, mesh.balls)
    {
        // get the parent ball
        if (ball.parentIndex == -1) {
            sweeps += Sweep(ball, ball);
        } else {
            const Ball &parent = mesh.balls[ball.parentIndex];
            sweeps += Sweep(ball, parent);
        }
    }
}

float MeshEvolution::scalarField(const Vector3 &pos) const
{
    float minPos = FLT_MAX, minNeg = 0;
    foreach (const Sweep &sweep, sweeps) {
        float curr = sweep.scalarField(pos);
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

void MeshEvolution::evolve() const
{
    for (int i = 0; i < mesh.vertices.count(); ++i) {
        Vertex &vertex = mesh.vertices[i];
        vertex.pos -= scalarFieldNormal(vertex.pos) * scalarField(vertex.pos);
    }
    mesh.updateNormals();
}

void MeshEvolution::run(Mesh &mesh)
{
    MeshEvolution evolution(mesh);
    evolution.evolve();
}
