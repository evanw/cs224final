#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "vector.h"
#include "mesh.h"

class Document;

class AddBallCommand : public QUndoCommand
{
private:
    Ball ball;
    Document *doc;

public:
    AddBallCommand(Document *doc, const Ball &ball);

    void undo();
    void redo();
};

class MoveBallCommand : public QUndoCommand
{
private:
    int index;
    Document *doc;
    Vector3 oldCenter, newCenter;

public:
    MoveBallCommand(Document *doc, int index, const Vector3 &delta);

    void undo();
    void redo();
};

class ScaleBallCommand : public QUndoCommand
{
private:
    int index;
    Document *doc;
    Vector3 oldX, newX;
    Vector3 oldY, newY;
    Vector3 oldZ, newZ;

public:
    ScaleBallCommand(Document *doc, int index, const Vector3 &x, const Vector3 &y, const Vector3 &z);

    void undo();
    void redo();
};

class DeleteBallCommand : public QUndoCommand
{
private:
    int index;
    QList<int> children;
    Document *doc;
    Ball ball;

public:
    DeleteBallCommand(Document *doc, int index);

    void undo();
    void redo();
};

class ChangeMeshCommand : public QUndoCommand
{
private:
    Document *doc;
    QVector<Vertex> oldVertices, newVertices;
    QVector<Triangle> oldTriangles, newTriangles;
    QVector<Quad> oldQuads, newQuads;

public:
    ChangeMeshCommand(Document *doc, const QVector<Vertex> &newVertices, const QVector<Triangle> &newTriangles, const QVector<Quad> &newQuads);

    void undo();
    void redo();
};

class ChangeVerticesCommand : public QUndoCommand
{
private:
    Document *doc;
    QVector<int> vertexIndices;
    QVector<Vertex> oldVertices, newVertices;

public:
    ChangeVerticesCommand(Document *doc, const QVector<int> &vertexIndices, const QVector<Vertex> &newVertices);

    void undo();
    void redo();
};

#endif // COMMANDS_H
