#include "jointrotation.h"
#include "view.h"

JointRotationTool::JointRotationTool(View *view) : Tool(view), baseMesh(view->doc->mesh.copy())
{
}


JointRotationTool::~JointRotationTool()
{
    delete baseMesh;
}



bool JointRotationTool::mousePressed(QMouseEvent *event)
{
    for (int i = 0; i < view->doc->mesh.vertices.size(); ++i) {
        Vertex &v = view->doc->mesh.vertices[i];
        v.pos.x += 0.5f;
        v.pos.y += 0.5f;
        v.pos.z += 0.5f;
    }

    return true;
}


void JointRotationTool::mouseDragged(QMouseEvent *event)
{

}


void JointRotationTool::mouseReleased(QMouseEvent *event)
{

}


void JointRotationTool::drawDebug(int, int)
{
    baseMesh->drawFill();
}
