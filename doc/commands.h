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

#endif // COMMANDS_H
