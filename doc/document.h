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

    void undo() { undoStack.undo(); }
    void redo() { undoStack.redo(); }
};

#endif // DOCUMENT_H
