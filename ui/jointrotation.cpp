#include "jointrotation.h"
#include "commands.h"
#include "view.h"
#include <QMouseEvent>
#include <QQuaternion>

static Vector3 getRotated(const Vector3 &vec, const QQuaternion &quat)
{
    QVector3D qvec(vec.x, vec.y, vec.z);
    const QVector3D &res = quat.rotatedVector(qvec);
    return Vector3(res.x(), res.y(), res.z());
}

JointRotationTool::JointRotationTool(View *view) : Tool(view), baseMesh(NULL)
{
    updateBaseMesh();
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
        baseMesh->updateChildIndices();

        // update the local and absolute transforms
        int n = baseMesh->vertices.count();
        relativeRotations.resize(n);
        relativeTranslations.resize(n);
        absoluteRotations.resize(n);
        absoluteTranslations.resize(n);
        calculateRelativeTransforms();
        calculateAbsoluteTransforms();

        meshInfo = newInfo;
    }
}

bool JointRotationTool::mousePressed(QMouseEvent *event)
{
    // Reset the base mesh
    meshInfo.reset();
    updateBaseMesh();

    view->selectedBall = getSelection(event->x(), event->y());
    if (view->selectedBall != -1) {
        originalRotation = relativeRotations[view->selectedBall];

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
        QQuaternion &rotation = relativeRotations[view->selectedBall];
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
    // Doing undo here results in pinching when you rotate somewhere and rotate back
    // This is the leasy hacky option but doesn't work well :(

    Mesh temp = view->doc->mesh;
    view->doc->mesh = *baseMesh;
    view->doc->getUndoStack().beginMacro("Rotate Joint");
    view->doc->changeMesh(temp.balls, temp.vertices, temp.triangles, temp.quads);
    view->doc->getUndoStack().endMacro();
}


void JointRotationTool::calculateRelativeTransforms()
{
    // calculate the offsets of the parents to the parents-of-parents
    for (int i = 0; i < baseMesh->balls.size(); ++i) {
        Ball &ball = baseMesh->balls[i];
        if (ball.parentIndex == -1) {
            relativeTranslations[i] = Vector3();
        } else {
            Ball &parent = baseMesh->balls[ball.parentIndex];
            if (parent.parentIndex == -1) {
                relativeTranslations[i] = parent.center;
            } else {
                relativeTranslations[i] = parent.center - baseMesh->balls[parent.parentIndex].center;
            }
        }
        relativeRotations[i] = QQuaternion();
    }
}


void JointRotationTool::calculateAbsoluteTransforms()
{
    foreach (int i, findRoots()) {
        calcTransform(i, QQuaternion(), Vector3());
    }
}

// recursively calculate absolute rotations for 'ball' and its children
void JointRotationTool::calcTransform(int i, const QQuaternion &parentRotation, const Vector3 &parentTranslation)
{
    absoluteTranslations[i] = parentTranslation + getRotated(relativeTranslations[i], parentRotation);
    absoluteRotations[i] = parentRotation * relativeRotations[i];

    foreach (int j, baseMesh->balls[i].childrenIndices) {
        calcTransform(j, absoluteRotations[i], absoluteTranslations[i]);
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
        ball.center = absoluteTranslations[index] + getRotated(relPos, absoluteRotations[index]);
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
        vert.pos = Vector3();
        QHashIterator<int, float> it(vert.jointWeights);
        while (it.hasNext()) {
            it.next();
            Ball &joint = baseMesh->balls[it.key()];
            Vector3 relPos = baseVert.pos - (joint.parentIndex == -1 ? joint.center : baseMesh->balls[joint.parentIndex].center);
            vert.pos += it.value() * (absoluteTranslations[it.key()] + getRotated(relPos, absoluteRotations[it.key()]));
        }
    }

    foreach (int i, findRoots())
        updateBallCenter(i);

    view->doc->mesh.updateNormals();
}
