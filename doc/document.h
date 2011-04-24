#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "mesh.h"
#include <QUndoStack>

/**
 * Document is the public interface to RawDocument that wraps it with undo support.
 */
class Document
{
private:
    QUndoStack undoStack;

public:
    Mesh mesh;

    QUndoStack &getUndoStack() { return undoStack; }

    void addBall(const Ball &ball);
    void moveBall(int ball, const Vector3 &delta);
    void deleteBall(int ball);
};

#endif // DOCUMENT_H
