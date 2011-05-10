#include "meshsculpter.h"
#include "view.h"
#include <QMouseEvent>

const float VOXEL_SPACING = 0.3f;

Vector3 MeshSculpterTool::interpolateAlongSnake(float t)
{
    t = max(0, min(1, t)) * snakePositions.count();
    int lo = max(0, min(t, snakePositions.count() - 1));
    int hi = max(0, min(lo + 1, snakePositions.count() - 1));
    return Vector3::lerp(snakePositions[lo], snakePositions[hi], t - lo);
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

void MeshSculpterTool::getVerticesInSphere(const Vector3 &center, float radius, QSet<MetaVertex *> &vertices)
{
    vertices.clear();

    // Query the acceleration data structure for vertices near the
    // brush. This may return vertices far away from the brush but
    // should be much faster than querying all of the vertices.
    accel->getVerticesInAABB(center - radius, center + radius, vertices);

    // Remove the vertices outside the brush radius
    float radiusSquared = radius * radius;
    QSet<MetaVertex *>::iterator i = vertices.begin();
    while (i != vertices.end())
    {
        float lengthSquared = ((*i)->pos - center).lengthSquared();
        if (lengthSquared > radiusSquared) i = vertices.erase(i);
        else i++;
    }
}

void MeshSculpterTool::stampBrush(const Vector3 &brushCenter, const Vector3 &brushNormal)
{
    QSet<MetaVertex *> brushVertices;
    getVerticesInSphere(brushCenter, brushRadius, brushVertices);

    // Move all vertices that are actually near the brush, and
    // update those in the acceleration data structure.
    QSet<Quad *> quadsNeedingNormals;
    QSet<MetaVertex *> movedVertices;
    foreach (MetaVertex *vertex, brushVertices)
    {
        float lengthSquared = (vertex->pos - brushCenter).lengthSquared();
        float percent = 1 - sqrtf(lengthSquared) / brushRadius;
        bool vertexChanged = false;

        // Move the vertex
        switch (brushMode)
        {
            case BRUSH_ADD_OR_SUBTRACT:
            {
                // Add or subtract material from the original vertex position along the
                // original vertex normal for consistency, which will add (or subtract)
                // a layer of consistent thickness
                float weight = percent * max(0, brushNormal.dot(vertex->prevNormal));
                if (weight > 0)
                {
                    Vector3 offset = vertex->prevNormal * (brushWeight * brushRadius);
                    if (isRightButton) offset = -offset;
                    vertex->pos = Vector3::lerp(vertex->pos, vertex->prevPos + offset, weight);
                    vertexChanged = true;
                }
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
                vertex->pos = Vector3::lerp(vertex->pos, target, brushWeight * percent);
                vertexChanged = true;
                break;
            }
        }

        if (vertexChanged)
        {
            movedVertices += vertex;

            // We'll need to recalculate normals for all neighboring quads
            foreach (Quad *quad, vertex->neighbors)
                quadsNeedingNormals += quad;
        }
    }
    accel->updateVertices(movedVertices);

    // Update normals and upload the result to the GPU
    commitChanges(quadsNeedingNormals);
}

void MeshSculpterTool::moveGrabbedVertices(int x, int y)
{
    Raytracer tracer;
    Vector3 origin = tracer.getEye();
    Vector3 ray = tracer.getRayForPixel(x, y);
    float t = grabbedNormal.dot(grabbedCenter - origin) / grabbedNormal.dot(ray);
    Vector3 hit = origin + ray * t;
    Vector3 delta = hit - grabbedCenter;
    Vector3 toCenter = (grabbedCenter - origin).unit();

    // Move all vertices that are actually near the brush, and
    // update those in the acceleration data structure.
    QSet<Quad *> quadsNeedingNormals;
    foreach (MetaVertex *vertex, grabbedVertices)
    {
        float lengthSquared = (vertex->prevPos - grabbedCenter).lengthSquared();
        float percent = max(0, 1 - sqrtf(lengthSquared) / brushRadius);
        percent = 0.5 - 0.5 * cosf(percent * M_PI);
        float mirroredPercent = 0;

        if (view->mirrorChanges)
        {
            float mirroredLengthSquared = (vertex->prevPos - grabbedCenter * Mesh::symmetryFlip).lengthSquared();
            mirroredPercent = max(0, 1 - sqrtf(mirroredLengthSquared) / brushRadius);
            mirroredPercent = 0.5 - 0.5 * cosf(mirroredPercent * M_PI);
        }

        Vector3 a, b;
        if (isRightButton)
        {
            a = vertex->prevPos + toCenter * ((hit.y - grabbedCenter.y) * percent);
            b = vertex->prevPos + toCenter * Mesh::symmetryFlip * ((hit.y - grabbedCenter.y) * mirroredPercent);
        }
        else
        {
            a = vertex->prevPos + delta * percent;
            b = vertex->prevPos + delta * Mesh::symmetryFlip * mirroredPercent;
        }
        if (percent + mirroredPercent > 1.0e-4f)
            vertex->pos = Vector3::lerp(a, b, mirroredPercent / (percent + mirroredPercent));

        // We'll need to recalculate normals for all neighboring quads
        foreach (Quad *quad, vertex->neighbors)
            quadsNeedingNormals += quad;
    }
    accel->updateVertices(grabbedVertices);

    // Update normals and upload the result to the GPU
    commitChanges(quadsNeedingNormals);
}

void MeshSculpterTool::moveSnake(int x, int y)
{
    Raytracer tracer;
    Vector3 origin = tracer.getEye();
    Vector3 ray = tracer.getRayForPixel(x, y);
    float t = grabbedNormal.dot(grabbedCenter - origin) / grabbedNormal.dot(ray);
    Vector3 hit = origin + ray * t;

    if (isRightButton)
    {
        // Move the hit point toward the camera
        float multiplier = (float)snakePositions.count() / (100 + snakePositions.count());
        hit += (origin - hit) * multiplier;
    }

    // Set the hit point as the head of the snake
    snakePositions += hit;

    // Move all vertices that are actually near the brush, and
    // update those in the acceleration data structure.
    QSet<Quad *> quadsNeedingNormals;
    foreach (MetaVertex *vertex, grabbedVertices)
    {
        float lengthSquared = (vertex->prevPos - grabbedCenter).lengthSquared();
        float percent = max(0, 1 - sqrtf(lengthSquared) / brushRadius);
        percent = 0.5 - 0.5 * cosf(percent * M_PI);
        float mirroredPercent = 0;

        if (view->mirrorChanges)
        {
            float mirroredLengthSquared = (vertex->prevPos - grabbedCenter * Mesh::symmetryFlip).lengthSquared();
            mirroredPercent = max(0, 1 - sqrtf(mirroredLengthSquared) / brushRadius);
            mirroredPercent = 0.5 - 0.5 * cosf(mirroredPercent * M_PI);
        }

        if (percent + mirroredPercent > 1.0e-4f)
        {
            Vector3 a = vertex->prevPos + (interpolateAlongSnake(percent) - grabbedCenter) * percent;
            Vector3 b = vertex->prevPos + (interpolateAlongSnake(mirroredPercent) - grabbedCenter) * Mesh::symmetryFlip * mirroredPercent;
            vertex->pos = Vector3::lerp(a, b, mirroredPercent / (percent + mirroredPercent));
        }

        // We'll need to recalculate normals for all neighboring quads
        foreach (Quad *quad, vertex->neighbors)
            quadsNeedingNormals += quad;
    }
    accel->updateVertices(grabbedVertices);

    // Update normals and upload the result to the GPU
    commitChanges(quadsNeedingNormals);
}

void MeshSculpterTool::commitChanges(QSet<Quad *> &quadsNeedingNormals)
{
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
        if (brushMode == BRUSH_GRAB || brushMode == BRUSH_SNAKE)
        {
            grabbedCenter = result.hit;
            grabbedNormal = tracer.getRayForPixel(view->width() / 2, view->height() / 2);
            getVerticesInSphere(grabbedCenter, brushRadius, grabbedVertices);
            if (view->mirrorChanges)
            {
                QSet<MetaVertex *> mirroredVertices;
                getVerticesInSphere(grabbedCenter * Mesh::symmetryFlip, brushRadius, mirroredVertices);
                grabbedVertices += mirroredVertices;
            }
            snakePositions.clear();
        }
        else
        {
            stampBrush(result.hit, result.normal);
            if (view->mirrorChanges) stampBrush(result.hit * Mesh::symmetryFlip, result.normal * Mesh::symmetryFlip);
        }
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
    if (brushMode == BRUSH_GRAB)
    {
        moveGrabbedVertices(event->x(), event->y());
    }
    else if (brushMode == BRUSH_SNAKE)
    {
        moveSnake(event->x(), event->y());
    }
    else if (accel->hitTest(tracer.getEye(), tracer.getRayForPixel(event->x(), event->y()), result))
    {
        stampBrush(result.hit, result.normal);
        if (view->mirrorChanges) stampBrush(result.hit * Mesh::symmetryFlip, result.normal * Mesh::symmetryFlip);
    }
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

        Vertex newVertex = vertex->wrappedVertex;
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
