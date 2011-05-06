#include "meshsculpter.h"
#include "view.h"

MeshSculpterTool::MeshSculpterTool(View *view) : Tool(view), mesh(view->getDocument().mesh), accel(new VoxelGrid(mesh, 0.1f)), brushRadius(0)
{
}

MeshSculpterTool::~MeshSculpterTool()
{
    delete accel;
}

bool MeshSculpterTool::mousePressed(QMouseEvent *event)
{
    return false;
}

void MeshSculpterTool::mouseDragged(QMouseEvent *event)
{
    Vector3 hit, normal; // TODO: get somehow
    float radiusSquared = brushRadius * brushRadius;

    // TODO: not O(n)
    for (int i = 0; i < mesh.vertices.count(); i++)
    {
        MetaVertex &vertex = *mesh.vertices[i];
        float lengthSquared = (vertex.pos - hit).lengthSquared();
        if (lengthSquared < radiusSquared)
        {
            float percent = sqrtf(lengthSquared) / brushRadius;
            vertex.pos += normal * (brushRadius * percent);
            vertex.marked = true;
        }
        else vertex.marked = false;
    }

//    // update normals
//    for (int i = 0; i < mesh.vertices.count(); i++)
//    {
//        MetaVertex &vertex = *mesh.vertices[i];
//        if (vertex.marked)
//            vertex.updateNormal();
//    }
}

void MeshSculpterTool::mouseReleased(QMouseEvent *event)
{
}
