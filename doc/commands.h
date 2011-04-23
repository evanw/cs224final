#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "vector.h"

class Document;

class MoveBallCommand : public QUndoCommand
{
private:
    int ball;
    Document *doc;
    Vector3 oldCenter, newCenter;

public:
    MoveBallCommand(int ball, Document *doc, const Vector3 &delta);

    void undo();
    void redo();
};

#endif // COMMANDS_H
