#include "meshsculpter.h"
#include "view.h"
#include <QMouseEvent>

const float VOXEL_SPACING = 0.3f;

MeshInfo::MeshInfo(Mesh &mesh) :
    isInitialized(true),
    ballPointer(mesh.balls.isEmpty() ? NULL : &mesh.balls[0]),
    vertexPointer(mesh.vertices.isEmpty() ? NULL : &mesh.vertices[0]),
    trianglePointer(mesh.triangles.isEmpty() ? NULL : &mesh.triangles[0]),
    quadPointer(mesh.quads.isEmpty() ? NULL : &mesh.quads[0]),
    ballCount(mesh.balls.count()),
    vertexCount(mesh.vertices.count()),
    triangleCount(mesh.triangles.count()),
    quadCount(mesh.quads.count())
{
}

bool MeshInfo::operator == (const MeshInfo &other) const
{
    return isInitialized && other.isInitialized &&
            ballPointer == other.ballPointer &&
            vertexPointer == other.vertexPointer &&
            trianglePointer == other.trianglePointer &&
            quadPointer == other.quadPointer &&
            ballCount == other.ballCount &&
            vertexCount == other.vertexCount &&
            triangleCount == other.triangleCount &&
            quadCount == other.quadCount;
}

void MeshSculpterTool::updateAccel()
{
    MeshInfo newInfo(view->doc->mesh);

    // If the mesh was changed, remake the acceleration data structure
    // because it holds pointers to the old mesh that no longer exists
    if (newInfo != meshInfo)
    {
        delete mesh;
        delete accel;

        mesh = new MetaMesh(view->doc->mesh);
        accel = new VoxelGrid(*mesh, VOXEL_SPACING);
        QObject::connect(view->doc, SIGNAL(verticesChanged(QVector<int>)), this, SLOT(verticesChanged(QVector<int>)));

        meshInfo = newInfo;

        // Also clear our array of vertices to commit because the
        // pointers to the vertices are no longer valid
        verticesToCommit.clear();
    }
}

void MeshSculpterTool::stampBrush(const Vector3 &brushCenter, const Vector3 &brushNormal)
{
    // Query the acceleration data structure for vertices near the
    // brush. This may return vertices far away from the brush but
    // should be much faster than querying all of the vertices.
    QSet<MetaVertex *> brushVertices;
    accel->getVerticesInAABB(brushCenter - brushRadius, brushCenter + brushRadius, brushVertices);

    // Move all vertices that are actually near the brush, and
    // update those in the acceleration data structure.
    QSet<Quad *> quadsNeedingNormals;
    QSet<MetaVertex *> movedVertices;
    float radiusSquared = brushRadius * brushRadius;
    foreach (MetaVertex *vertex, brushVertices)
    {
        float lengthSquared = (vertex->pos - brushCenter).lengthSquared();
        if (lengthSquared < radiusSquared)
        {
            float percent = 1 - sqrtf(lengthSquared) / brushRadius;
            float weight = percent * max(0, brushNormal.dot(vertex->prevNormal));

            if (weight > 0)
            {
                // Move the vertex
                switch (brushMode)
                {
                    case BRUSH_ADD_OR_SUBTRACT:
                    {
                        // Add or subtract material from the original vertex position along the
                        // original vertex normal for consistency, which will add (or subtract)
                        // a layer of consistent thickness
                        Vector3 offset = vertex->prevNormal * (brushWeight * brushRadius);
                        if (isRightButton) offset = -offset;
                        vertex->pos = Vector3::lerp(vertex->pos, vertex->prevPos + offset, weight);
                        break;
                    }

                    case BRUSH_SMOOTH:
                    {
                        // Compute the average neighbor center (this is gross and order dependent
                        // because we are modifying the vertices as we iterate over them)
                        Vector3 average;
                        foreach (Quad *quad, vertex->neighbors)
                        {
                            Vector3 &a = mesh->vertices[quad->a.index]->pos;
                            Vector3 &b = mesh->vertices[quad->b.index]->pos;
                            Vector3 &c = mesh->vertices[quad->c.index]->pos;
                            Vector3 &d = mesh->vertices[quad->d.index]->pos;
                            average += (a + b + c + d) / 4;
                        }
                        average /= vertex->neighbors.count();

                        // Project the average onto the tangent plane
                        Vector3 target = average + vertex->normal * vertex->normal.dot(average - vertex->pos);
                        vertex->pos = Vector3::lerp(vertex->pos, target, brushWeight * weight);
                        break;
                    }
                }
                movedVertices += vertex;

                // We'll need to recalculate normals for all neighboring quads
                foreach (Quad *quad, vertex->neighbors)
                    quadsNeedingNormals += quad;
            }
        }
    }
    accel->updateVertices(movedVertices);

    // Update the normals for all vertices touching a moved quad
    QSet<MetaVertex *> verticesNeedingNormals;
    foreach (Quad *quad, quadsNeedingNormals)
    {
        verticesNeedingNormals += mesh->vertices[quad->a.index];
        verticesNeedingNormals += mesh->vertices[quad->b.index];
        verticesNeedingNormals += mesh->vertices[quad->c.index];
        verticesNeedingNormals += mesh->vertices[quad->d.index];
    }
    foreach (MetaVertex *vertex, verticesNeedingNormals)
    {
        foreach (Quad *quad, vertex->neighbors)
            quadsNeedingNormals += quad;
        vertex->normal = Vector3();
    }
    foreach (Quad *quad, quadsNeedingNormals)
    {
        MetaVertex *a = mesh->vertices[quad->a.index];
        MetaVertex *b = mesh->vertices[quad->b.index];
        MetaVertex *c = mesh->vertices[quad->c.index];
        MetaVertex *d = mesh->vertices[quad->d.index];
        Vector3 normal = (
                (b->pos - a->pos).cross(d->pos - a->pos) +
                (c->pos - b->pos).cross(a->pos - b->pos) +
                (d->pos - c->pos).cross(b->pos - c->pos) +
                (a->pos - d->pos).cross(c->pos - d->pos)
            ).unit();
        if (verticesNeedingNormals.contains(a)) a->normal += normal;
        if (verticesNeedingNormals.contains(b)) b->normal += normal;
        if (verticesNeedingNormals.contains(c)) c->normal += normal;
        if (verticesNeedingNormals.contains(d)) d->normal += normal;
    }
    foreach (MetaVertex *vertex, verticesNeedingNormals)
    {
        vertex->normal.normalize();
        verticesToCommit += vertex;
    }

    // Commit the result to the GPU
    mesh->mesh.uploadToGPU();
}

MeshSculpterTool::MeshSculpterTool(View *view) :
    Tool(view), mesh(NULL), accel(NULL), isRightButton(false),
    brushRadius(0), brushWeight(0), brushMode(BRUSH_ADD_OR_SUBTRACT)
{
}

MeshSculpterTool::~MeshSculpterTool()
{
    delete mesh;
    delete accel;
}

void MeshSculpterTool::drawDebug(int x, int y)
{
#ifdef VOXEL_DEBUG
    glColor4f(0, 0, 0, 0.5f);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    updateAccel();
    accel->drawDebug();

    Raytracer tracer;
    HitTest result;
    accel->hitTest(tracer.getEye(), tracer.getRayForPixel(x, y), result);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
#endif
}

bool MeshSculpterTool::mousePressed(QMouseEvent *event)
{
    updateAccel();
    isRightButton = (event->button() == Qt::RightButton);

    view->camera3D();
    Raytracer tracer;
    HitTest result;
    if (accel->hitTest(tracer.getEye(), tracer.getRayForPixel(event->x(), event->y()), result))
    {
        stampBrush(result.hit, result.normal);
        return true;
    }

    return false;
}

void MeshSculpterTool::mouseDragged(QMouseEvent *event)
{
    updateAccel();

    view->camera3D();
    Raytracer tracer;
    HitTest result;
    if (accel->hitTest(tracer.getEye(), tracer.getRayForPixel(event->x(), event->y()), result))
        stampBrush(result.hit, result.normal);
}

void MeshSculpterTool::mouseReleased(QMouseEvent *)
{
    updateAccel();

    // Don't add a no-op to the undo history
    if (verticesToCommit.isEmpty())
        return;

    // Map vertex pointers to their indices
    QHash<MetaVertex *, int> map;
    for (int i = 0; i < mesh->vertices.count(); i++)
        map[mesh->vertices[i]] = i;

    // Generate the info for all vertices in verticesToCommit
    QVector<int> vertexIndices;
    QVector<Vertex> newVertices;
    foreach (MetaVertex *vertex, verticesToCommit)
    {
        vertexIndices += map[vertex];

        Vertex newVertex;
        newVertex.pos = vertex->pos;
        newVertex.normal = vertex->normal;
        vertex->pos = vertex->prevPos;
        vertex->normal = vertex->prevNormal;
        newVertices += newVertex;
    }

    // Add the command to the undo stack
    view->doc->getUndoStack().beginMacro("Change Vertices");
    view->doc->changeVertices(vertexIndices, newVertices);
    view->doc->getUndoStack().endMacro();

    // Get ready for the next brush stroke
    foreach (MetaVertex *vertex, verticesToCommit)
    {
        vertex->prevPos = vertex->pos;
        vertex->prevNormal = vertex->normal;
    }
    verticesToCommit.clear();
}

void MeshSculpterTool::verticesChanged(const QVector<int> &vertexIndices)
{
    updateAccel();

    // Update the previous per-vertex information
    foreach (int index, vertexIndices)
    {
        MetaVertex *vertex = mesh->vertices[index];
        vertex->prevPos = vertex->pos;
        vertex->prevNormal = vertex->normal;
    }

    // Also update the acceleration data structure
    QSet<MetaVertex *> changedVertices;
    foreach (int index, vertexIndices)
        changedVertices += mesh->vertices[index];
    accel->updateVertices(changedVertices);
}
