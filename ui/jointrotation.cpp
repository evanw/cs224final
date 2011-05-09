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

// Convert to Matrix
/* QMatrix4x4 getMatrix(const QQuaternion &quat) const
{


        float x2 = x * x;
        float y2 = y * y;
        float z2 = z * z;
        float xy = x * y;
        float xz = x * z;
        float yz = y * z;
        float wx = w * x;
        float wy = w * y;
        float wz = w * z;

        // This calculation would be a lot more complicated for non-unit length quaternions
        // Note: The constructor of Matrix4 expects the Matrix in column-major format like expected by
        //   OpenGL
        return Matrix4( 1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
                        2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
                        2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f)
} */


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
        for (int j = 0; j < 2; ++j) {
            int ijoint = vert.jointIndices[j];
            if (ijoint != -1) {
                float weight = vert.jointWeights[j];
                Ball *joint = &view->doc->mesh.balls[ijoint];
                Vector3 relPos = baseVert.pos - joint->center;
                pos += weight * (absoluteTransforms[joint] * QVector3D(relPos.x, relPos.y, relPos.z));
            }
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
