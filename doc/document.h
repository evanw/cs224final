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
    Mesh raw;

    QUndoStack &getUndoStack() { return undoStack; }
};

#endif // DOCUMENT_H
