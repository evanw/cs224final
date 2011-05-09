#include "jointrotation.h"
#include "view.h"
#include <QMouseEvent>
#include <QQuaternion>
#include <QMatrix4x4>

static Vector3 getRotated(const Vector3 &vec, const QQuaternion &quat)
{
    QVector3D qvec(vec.x, vec.y, vec.z);
    const QVector3D &res = quat.rotatedVector(qvec);
    return Vector3(res.x(), res.y(), res.z());
}


JointRotationTool::JointRotationTool(View *view) : Tool(view), baseMesh(view->doc->mesh.copy())
{
    view->doc->mesh.updateChildIndices();
    view->doc->mesh.updateRelativePositions();
}


JointRotationTool::~JointRotationTool()
{
    delete baseMesh;
}


bool JointRotationTool::mousePressed(QMouseEvent *event)
{
    // TODO: When do I create the base mesh and make sure children indices are updated?
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
            calculateAbsoluteTransforms();
            updateVertices();
            //view->doc->getUndoStack().endMacro();
        }
    }
}


void JointRotationTool::calculateAbsoluteTransforms()
{
    QList<Ball *> roots;
    // get all the roots
    for (int i = 0; i < view->doc->mesh.balls.size(); ++i) {
        Ball &ball = view->doc->mesh.balls[i];
        if (ball.parentIndex == -1) {
            roots += &ball;
        }
    }

    for (int i = 0; i < roots.size(); ++i) {
        calcTransform(roots[i], QMatrix4x4());
    }
}

// recursively calculate absolute rotations for 'ball' and its children
void JointRotationTool::calcTransform(Ball *ball, QMatrix4x4 currTransform)
{
    currTransform.translate(ball->translation.x, ball->translation.y, ball->translation.z);
    currTransform.rotate(ball->rotation);
    absoluteTransforms[ball] = currTransform;

    for (int i = 0; i < ball->childrenIndices.size(); ++i) {
        calcTransform(&view->doc->mesh.balls[ball->childrenIndices[i]], currTransform);
    }
}


void JointRotationTool::updateVertices()
{
    for (int i = 0; i < view->doc->mesh.vertices.size(); ++i) {
        Vertex &vert = view->doc->mesh.vertices[i];
        const Vertex &baseVert = baseMesh->vertices[i];

        // calculate affine combination of rotated positions
        QVector3D pos;
        QHashIterator<int, float> it(vert.jointWeights);
        while (it.hasNext()) {
            it.next();
            Ball *joint = &view->doc->mesh.balls[it.key()];
            Vector3 relPos = baseVert.pos - joint->center;
            QVector3D qRelPos(relPos.x, relPos.y, relPos.z);
            pos += it.value() * (absoluteTransforms[joint] * qRelPos);
        }
        vert.pos = Vector3(pos.x(), pos.y(), pos.z());
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
