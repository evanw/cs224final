#include "jointrotation.h"
#include "commands.h"
#include "view.h"
#include <QMouseEvent>
#include <QQuaternion>

// Define ANIMATION_UNDO to integrate animation with undo. Doing undo while posing
// results in pinching when you rotate somewhere and rotate back. This is mostly
// a result of adding posing as an afterthought at the end of the project. I'm
// disabling undo for animation at the moment for a better final project demo.
// #define ANIMATION_UNDO

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
#ifdef ANIMATION_UNDO
    // Reset the base mesh
    meshInfo.reset();
#endif
    updateBaseMesh();

    oldX = event->x();
    oldY = event->y();

    HitTest result;
    if (view->selectedBall == -1 || !hitTestSelection(event->x(), event->y(), result, METHOD_CUBE))
    {
        view->selectedBall = getSelection(event->x(), event->y());
        if (view->selectedBall == -1) return false;
        hitTestSelection(event->x(), event->y(), result, METHOD_CUBE);
    }

    originalRotation = relativeRotations[view->selectedBall];
    Ball &ball = view->doc->mesh.balls[view->selectedBall];
    Vector3 center = ball.parentIndex == -1 ? ball.center : view->doc->mesh.balls[ball.parentIndex].center;
    projectedCenter = center + result.normal * result.normal.dot(result.hit - center);
    planeNormal = result.normal;
    originalAngle = getAngleOnPlane(event->x(), event->y());
    return true;
}


void JointRotationTool::mouseDragged(QMouseEvent *event)
{
    updateBaseMesh();

    if (view->selectedBall != -1)
    {
        float deltaAngle = getAngleOnPlane(event->x(), event->y()) - originalAngle;
        Ball &ball = view->doc->mesh.balls[view->selectedBall];
        QQuaternion parentRotation = (ball.parentIndex == -1) ? QQuaternion() : absoluteRotations[ball.parentIndex];
        QQuaternion inverseParentRotation(parentRotation.scalar(), -parentRotation.vector());

        QQuaternion deltaRotation = QQuaternion::fromAxisAndAngle(planeNormal.x, planeNormal.y, planeNormal.z, deltaAngle);
        QQuaternion &rotation = relativeRotations[view->selectedBall];
        rotation = (inverseParentRotation * deltaRotation * parentRotation) * originalRotation;
        rotation.normalize();

        calculateAbsoluteTransforms();
        updateVertices();

        oldX = event->x();
        oldY = event->y();
    }
}


void JointRotationTool::mouseReleased(QMouseEvent *)
{
#ifdef ANIMATION_UNDO
    // Doing undo here results in pinching when you rotate somewhere and rotate back
    // This is the least hacky option but doesn't work well :(

    Mesh temp = view->doc->mesh;
    view->doc->mesh = *baseMesh;
    view->doc->getUndoStack().beginMacro("Rotate Joint");
    view->doc->changeMesh(temp.balls, temp.vertices, temp.triangles, temp.quads);
    view->doc->getUndoStack().endMacro();
#endif
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

float JointRotationTool::getAngleOnPlane(int x, int y)
{
    view->camera3D();
    Raytracer tracer;
    Vector3 origin = tracer.getEye();
    Vector3 ray = tracer.getRayForPixel(x, y);
    float t = planeNormal.dot(projectedCenter - origin) / planeNormal.dot(ray);
    Vector3 delta = origin + ray * t - projectedCenter;
    Vector3 axisX = ((fabsf(planeNormal.dot(Vector3::X)) < 0.75) ? Vector3::X : Vector3::Y).cross(planeNormal).unit();
    Vector3 axisY = planeNormal.cross(axisX).unit();
    return atan2f(delta.dot(axisY), delta.dot(axisX)) * 180 / M_PI;
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
