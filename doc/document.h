#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "mesh.h"
#include <QUndoStack>

/**
 * Document is the public interface to RawDocument that wraps it with undo support.
 */
class Document : public QObject
{
    Q_OBJECT

private:
    QUndoStack undoStack;

public:
    Mesh mesh;

    QUndoStack &getUndoStack() { return undoStack; }

    void addBall(const Ball &ball);
    void moveBall(int index, const Vector3 &delta);
    void scaleBall(int index, const Vector3 &x, const Vector3 &y, const Vector3 &z);
    void deleteBall(int index);
    void changeMesh(const QVector<Vertex> &vertices, const QVector<Triangle> &triangles, const QVector<Quad> &quads);

    void emitDocumentChanged() { emit documentChanged(); }

signals:
    void documentChanged();
};

#endif // DOCUMENT_H
