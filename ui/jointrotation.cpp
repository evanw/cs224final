#include "jointrotation.h"
#include "view.h"
#include <QMouseEvent>
#include <QQuaternion>
#include <QMatrix4x4>

JointRotationTool::JointRotationTool(View *view) : Tool(view), baseMesh(NULL)
{
    updateBaseMesh();
    baseMesh->updateChildIndices();
    calculateRelativePositions();
}

JointRotationTool::~JointRotationTool()
{
    delete baseMesh;
}

QList<int> JointRotationTool::findRoots()
{
    QList<int> roots;
    for (int i = 0; i < view->doc->mesh.balls.size(); ++i) {
        Ball &ball = view->doc->mesh.balls[i];
        if (ball.parentIndex == -1) {
            roots += i;
        }
    }
    return roots;
}

void JointRotationTool::updateBaseMesh()
{
    MeshInfo newInfo(view->doc->mesh);

    // If the mesh was changed, remake the base mesh because the sizes could have changed
    if (newInfo != meshInfo)
    {
        delete baseMesh;
        baseMesh = view->doc->mesh.copy();

        // update the local and absolute transforms
        int n = baseMesh->vertices.count();
        rotations.resize(n);
        translations.resize(n);
        absoluteTransforms.resize(n);
        calculateAbsoluteTransforms();

        meshInfo = newInfo;
    }
}

bool JointRotationTool::mousePressed(QMouseEvent *event)
{
    updateBaseMesh();

    view->selectedBall = getSelection(event->x(), event->y());
    if (view->selectedBall != -1) {
        originalRotation = rotations[view->selectedBall];

        oldX = event->x();
        oldY = event->y();
        return true;
    }

    return false;
}


void JointRotationTool::mouseDragged(QMouseEvent *event)
{
    updateBaseMesh();

    if (view->selectedBall != -1) {
        QQuaternion &rotation = rotations[view->selectedBall];
        rotation *= QQuaternion(1, (event->y() - oldY) * 0.01f, 0, 0);
        rotation.normalize();

        calculateAbsoluteTransforms();
        updateVertices();

        oldX = event->x();
        oldY = event->y();
    }
}


void JointRotationTool::mouseReleased(QMouseEvent *)
{
}


void JointRotationTool::calculateRelativePositions()
{
    // calculate the offsets of the parents to the parents-of-parents
    for (int i = 0; i < baseMesh->balls.size(); ++i) {
        Ball &ball = baseMesh->balls[i];
        if (ball.parentIndex == -1) {
            translations[i] = ball.center;
        } else {
            Ball &parent = baseMesh->balls[ball.parentIndex];
            if (parent.parentIndex == -1) {
                translations[i] = parent.center;
            } else {
                translations[i] = parent.center - baseMesh->balls[parent.parentIndex].center;
            }
        }
    }
}


void JointRotationTool::calculateAbsoluteTransforms()
{
    foreach (int i, findRoots()) {
        calcTransform(i, QMatrix4x4());
    }
}

// recursively calculate absolute rotations for 'ball' and its children
void JointRotationTool::calcTransform(int i, QMatrix4x4 currTransform)
{
    currTransform.translate(translations[i].x, translations[i].y, translations[i].z);
    currTransform.rotate(rotations[i]);
    absoluteTransforms[i] = currTransform;

    foreach (int j, baseMesh->balls[i].childrenIndices) {
        calcTransform(j, currTransform);
    }
}


void JointRotationTool::updateBallCenter(int index)
{
    Ball &ball = view->doc->mesh.balls[index];
    Ball &baseBall = baseMesh->balls[index];

    if (ball.parentIndex != -1)
    {
        Ball &baseParent = baseMesh->balls[ball.parentIndex];
        Vector3 relPos = baseBall.center - baseParent.center;
        QVector3D qRelPos(relPos.x, relPos.y, relPos.z);
        QVector3D newCenter = absoluteTransforms[index] * qRelPos;
        ball.center = Vector3(newCenter.x(), newCenter.y(), newCenter.z());
    }
    else ball.center = baseBall.center;

    foreach (int i, baseBall.childrenIndices)
        updateBallCenter(i);
}


void JointRotationTool::updateVertices()
{
    for (int i = 0; i < baseMesh->vertices.count(); i++) {
        Vertex &vert = view->doc->mesh.vertices[i];
        const Vertex &baseVert = baseMesh->vertices[i];

        // calculate affine combination of rotated positions
        QVector3D pos;
        QHashIterator<int, float> it(vert.jointWeights);
        while (it.hasNext()) {
            it.next();
            Ball &joint = baseMesh->balls[it.key()];
            Vector3 relPos = baseVert.pos - (joint.parentIndex == -1 ? joint.center : baseMesh->balls[joint.parentIndex].center);
            QVector3D qRelPos(relPos.x, relPos.y, relPos.z);
            pos += it.value() * (absoluteTransforms[it.key()] * qRelPos);
        }
        vert.pos = Vector3(pos.x(), pos.y(), pos.z());
    }

    foreach (int i, findRoots())
        updateBallCenter(i);

    view->doc->mesh.updateNormals();
}
