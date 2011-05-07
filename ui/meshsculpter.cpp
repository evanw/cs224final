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

        meshInfo = newInfo;
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
    QSet<Quad *> movedQuads;
    QSet<MetaVertex *> movedVertices;
    float radiusSquared = brushRadius * brushRadius;
    foreach (MetaVertex *vertex, brushVertices)
    {
        float lengthSquared = (vertex->pos - brushCenter).lengthSquared();
        if (lengthSquared < radiusSquared)
        {
            // float percent = 1 - sqrtf(lengthSquared) / brushRadius;
            float weight = vertex->normal.dot(brushNormal) * brushRadius * 0.05f;
            vertex->pos += brushNormal * weight;
            movedVertices += vertex;

            // We'll need to recalculate normals for all neighboring quads
            foreach (Quad *quad, vertex->neighbors)
                movedQuads += quad;
        }
    }
    accel->updateVertices(movedVertices);

    // Update the normals for all vertices touching a moved quad
    QSet<MetaVertex *> verticesNeedingNormals;
    foreach (Quad *quad, movedQuads)
    {
        verticesNeedingNormals += mesh->vertices[quad->a.index];
        verticesNeedingNormals += mesh->vertices[quad->b.index];
        verticesNeedingNormals += mesh->vertices[quad->c.index];
        verticesNeedingNormals += mesh->vertices[quad->d.index];
    }
    foreach (MetaVertex *vertex, verticesNeedingNormals)
        vertex->normal = Vector3();
    foreach (Quad *quad, movedQuads)
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
        vertex->normal.normalize();
}

MeshSculpterTool::MeshSculpterTool(View *view) : Tool(view), mesh(NULL), accel(NULL), brushRadius(0)
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
    // accel->drawDebug();

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
    // TODO: undo
    updateAccel();
}
