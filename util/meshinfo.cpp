#include "meshinfo.h"

MeshInfo::MeshInfo(Mesh &mesh) :
    isInitialized(true),
    ballPointer(mesh.balls.isEmpty() ? NULL : &mesh.balls[0]),
    vertexPointer(mesh.vertices.isEmpty() ? NULL : &mesh.vertices[0]),
    trianglePointer(mesh.triangles.isEmpty() ? NULL : &mesh.triangles[0]),
    quadPointer(mesh.quads.isEmpty() ? NULL : &mesh.quads[0]),
    ballCount(mesh.balls.count()),
    vertexCount(mesh.vertices.count()),
    triangleCount(mesh.triangles.count()),
    quadCount(mesh.quads.count())
{
}

bool MeshInfo::operator == (const MeshInfo &other) const
{
    return isInitialized && other.isInitialized &&
            ballPointer == other.ballPointer &&
            vertexPointer == other.vertexPointer &&
            trianglePointer == other.trianglePointer &&
            quadPointer == other.quadPointer &&
            ballCount == other.ballCount &&
            vertexCount == other.vertexCount &&
            triangleCount == other.triangleCount &&
            quadCount == other.quadCount;
}
