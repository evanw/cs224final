#include "commands.h"
#include "document.h"
#include <assert.h>

AddBallCommand::AddBallCommand(Document *doc, const Ball &ball) : ball(ball), doc(doc)
{
}

void AddBallCommand::undo()
{
    doc->mesh.balls.pop_back();
    doc->emitDocumentChanged();
}

void AddBallCommand::redo()
{
    doc->mesh.balls += ball;
    doc->emitDocumentChanged();
}

MoveBallCommand::MoveBallCommand(Document *doc, int index, const Vector3 &delta) : index(index), doc(doc)
{
    oldCenter = doc->mesh.balls[index].center;
    newCenter = oldCenter + delta;
}

void MoveBallCommand::undo()
{
    doc->mesh.balls[index].center = oldCenter;
    doc->emitDocumentChanged();
}

void MoveBallCommand::redo()
{
    doc->mesh.balls[index].center = newCenter;
    doc->emitDocumentChanged();
}

ScaleBallCommand::ScaleBallCommand(Document *doc, int index, const Vector3 &x, const Vector3 &y, const Vector3 &z) : index(index), doc(doc)
{
    Ball &ball = doc->mesh.balls[index];
    oldX = ball.ex;
    oldY = ball.ey;
    oldZ = ball.ez;
    newX = x;
    newY = y;
    newZ = z;
}

void ScaleBallCommand::undo()
{
    Ball &ball = doc->mesh.balls[index];
    ball.ex = oldX;
    ball.ey = oldY;
    ball.ez = oldZ;
    doc->emitDocumentChanged();
}

void ScaleBallCommand::redo()
{
    Ball &ball = doc->mesh.balls[index];
    ball.ex = newX;
    ball.ey = newY;
    ball.ez = newZ;
    doc->emitDocumentChanged();
}

DeleteBallCommand::DeleteBallCommand(Document *doc, int index) : index(index), doc(doc), ball(doc->mesh.balls[index])
{
}

void DeleteBallCommand::undo()
{
    // add one to all indices >= index and connect child bones
    for (int i = 0; i < doc->mesh.balls.count(); i++)
    {
        Ball &ball = doc->mesh.balls[i];
        if (ball.parentIndex >= index)
            ball.parentIndex++;
    }
    foreach (int child, children)
        doc->mesh.balls[child].parentIndex = index;

    // can now modify the list
    doc->mesh.balls.insert(index, ball);
    doc->emitDocumentChanged();
}

void DeleteBallCommand::redo()
{
    // subtract one from all indices > index and disconnect child bones
    for (int i = 0; i < doc->mesh.balls.count(); i++)
    {
        Ball &ball = doc->mesh.balls[i];
        if (ball.parentIndex == index)
        {
            ball.parentIndex = -1;
            children += (i > index) ? i - 1 : i;
        }
        else if (ball.parentIndex > index)
            ball.parentIndex--;
    }

    // can now modify the list
    doc->mesh.balls.remove(index);
    doc->emitDocumentChanged();
}

ChangeMeshCommand::ChangeMeshCommand(Document *doc, const QVector<Ball> &balls, const QVector<Vertex> &vertices, const QVector<Triangle> &triangles, const QVector<Quad> &quads)
    : doc(doc),
      oldBalls(doc->mesh.balls), newBalls(balls),
      oldVertices(doc->mesh.vertices), newVertices(vertices),
      oldTriangles(doc->mesh.triangles), newTriangles(triangles),
      oldQuads(doc->mesh.quads), newQuads(quads)
{
}

void ChangeMeshCommand::undo()
{
    doc->mesh.balls = oldBalls;
    doc->mesh.vertices = oldVertices;
    doc->mesh.triangles = oldTriangles;
    doc->mesh.quads = oldQuads;
    doc->mesh.uploadToGPU();
    doc->emitDocumentChanged();
}

void ChangeMeshCommand::redo()
{
    doc->mesh.balls = newBalls;
    doc->mesh.vertices = newVertices;
    doc->mesh.triangles = newTriangles;
    doc->mesh.quads = newQuads;
    doc->mesh.uploadToGPU();
    doc->emitDocumentChanged();
}

ChangeVerticesCommand::ChangeVerticesCommand(Document *doc, const QVector<int> &vertexIndices, const QVector<Vertex> &newVertices)
    : doc(doc),
      vertexIndices(vertexIndices),
      newVertices(newVertices)
{
    assert(vertexIndices.count() == newVertices.count());

    foreach (int index, vertexIndices)
        oldVertices += doc->mesh.vertices[index];
}

void ChangeVerticesCommand::undo()
{
    for (int i = 0; i < vertexIndices.count(); i++)
        doc->mesh.vertices[vertexIndices[i]] = oldVertices[i];
    doc->emitDocumentChanged();
    doc->emitVerticesChanged(vertexIndices);
    doc->mesh.uploadToGPU();
}

void ChangeVerticesCommand::redo()
{
    for (int i = 0; i < vertexIndices.count(); i++)
        doc->mesh.vertices[vertexIndices[i]] = newVertices[i];
    doc->emitDocumentChanged();
    doc->emitVerticesChanged(vertexIndices);
    doc->mesh.uploadToGPU();
}
