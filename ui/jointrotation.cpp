#include "jointrotation.h"
#include "view.h"
#include <QMouseEvent>
#include <QQuaternion>

static Vector3 getRotated(const Vector3 &vec, const QQuaternion &quat)
{
    QVector3D qvec(vec.x, vec.y, vec.z);
    const QVector3D &res = quat.rotatedVector(qvec);
    return Vector3(res.x(), res.y(), res.z());
}


JointRotationTool::JointRotationTool(View *view) : Tool(view), baseMesh(view->doc->mesh.copy())
{
}


JointRotationTool::~JointRotationTool()
{
    delete baseMesh;
}


bool JointRotationTool::mousePressed(QMouseEvent *event)
{
    // TODO: When do I create the base mesh?
    //delete baseMesh;
    //baseMesh = view->doc->mesh.copy();

    view->selectedBall = getSelection(event->x(), event->y());
    if (view->selectedBall != -1) {
        Ball &ball = view->doc->mesh.balls[view->selectedBall];
        originalRotation = ball.rotation;
        return true;
    }

    return false;
}


void JointRotationTool::mouseDragged(QMouseEvent *event)
{
    if (view->selectedBall != -1) {
        Ball &ball = view->doc->mesh.balls[view->selectedBall];
        ball.rotation *= QQuaternion(1, .01, 0, 0);
        ball.rotation.normalize();
    }
}


void JointRotationTool::mouseReleased(QMouseEvent *event)
{
    if (view->selectedBall != -1) {
        Ball &ball = view->doc->mesh.balls[view->selectedBall];
        QQuaternion newRotation = ball.rotation;
        ball.rotation = originalRotation;

        if (ball.rotation != newRotation) {
            //view->doc->getUndoStack().beginMacro("Rotate bone");
            // TODO: Use an undo command here
            ball.rotation = newRotation;
            calculateAbsoluteRotations();
            updateVertices();
            //view->doc->getUndoStack().endMacro();
        }
    }
}


void JointRotationTool::calculateAbsoluteRotations()
{

}


void JointRotationTool::updateVertices()
{
    for (int i = 0; i < view->doc->mesh.vertices.size(); ++i) {
        Vertex &vert = view->doc->mesh.vertices[i];
        const Vertex &baseVert = baseMesh->vertices[i];

        // calculate affine combination of rotated positions
        vert.pos.x = vert.pos.y = vert.pos.z = 0;
        for (int j = 0; j < 2; ++j) {
            int ijoint = vert.jointIndices[j];
            if (ijoint != -1) {
                float weight = vert.jointWeights[j];
                const Ball &joint = view->doc->mesh.balls[ijoint];
                vert.pos += weight * (joint.center + getRotated(baseVert.pos - joint.center, joint.rotation));
            }
        }
    }

    view->doc->mesh.updateNormals();
}


void JointRotationTool::drawDebug(int, int)
{
    // draw initial mesh
    glColor3f(0.25f, 0.25f, 0.85f);
    glEnable(GL_LIGHTING);
    glEnable(GL_POLYGON_OFFSET_FILL);
    baseMesh->drawFill();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_LIGHTING);
    glColor3f(0, 0, 0);
}
