#ifndef MESHSCULPTER_H
#define MESHSCULPTER_H

#include "tools.h"
#include "meshacceleration.h"

enum { MESH_GRAB };

class MeshInfo
{
private:
    bool isInitialized;
    Ball *ballPointer;
    Vertex *vertexPointer;
    Triangle *trianglePointer;
    Quad *quadPointer;
    int ballCount;
    int vertexCount;
    int triangleCount;
    int quadCount;

public:
    MeshInfo() : isInitialized(false) {}
    MeshInfo(Mesh &mesh);

    bool operator == (const MeshInfo &other) const;
    bool operator != (const MeshInfo &other) const { return !(*this == other); }
};

enum { BRUSH_ADD_OR_SUBTRACT, BRUSH_SMOOTH };

/**
 * MeshSculpterTool must derive from QObject to use the verticesChanged function.
 * However, it cannot use multiple inheritance (even if QObject is the first
 * superclass as the documentation says) because when trying to delete a
 * MeshSculpterTool the allocator exits with the error "pointer being freed
 * was not allocated". The solution is to make Tool inherit from QObject instead,
 * which only uses single inheritance.
 */
class MeshSculpterTool : public Tool
{
    Q_OBJECT

private:
    MetaMesh *mesh;
    AccelerationDataStructure *accel;
    bool isRightButton;

    // Remember info about the mesh so we can tell when it has changed
    MeshInfo meshInfo;

    // List of vertices that will be added to the ChangeVerticesCommand on mouse up
    QSet<MetaVertex *> verticesToCommit;

    void updateAccel();
    void stampBrush(const Vector3 &brushCenter, const Vector3 &brushNormal);

public:
    float brushRadius;
    float brushWeight;
    int brushMode;

    MeshSculpterTool(View *view);
    ~MeshSculpterTool();

    void drawDebug(int x, int y);
    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);

public slots:
    void verticesChanged(const QVector<int> &vertexIndices);
};

#endif // MESHSCULPTER_H
