#include "document.h"
#include "commands.h"

void Document::addBall(const Ball &ball)
{
    undoStack.push(new AddBallCommand(this, ball));
}

void Document::moveBall(int index, const Vector3 &delta)
{
    if (delta.lengthSquared() > 0)
        undoStack.push(new MoveBallCommand(this, index, delta));
}

void Document::scaleBall(int index, const Vector3 &x, const Vector3 &y, const Vector3 &z)
{
    Ball &ball = mesh.balls[index];
    if (x != ball.ex || y != ball.ey || z != ball.ez)
        undoStack.push(new ScaleBallCommand(this, index, x, y, z));
}

void Document::deleteBall(int index)
{
    undoStack.push(new DeleteBallCommand(this, index));
}

void Document::changeMesh(const QVector<Vertex> &vertices, const QVector<Triangle> &triangles, const QVector<Quad> &quads)
{
    undoStack.push(new ChangeMeshCommand(this, vertices, triangles, quads));
}
