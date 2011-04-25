#include "tools.h"
#include "view.h"
#include "selectionrecorder.h"
#include <QMouseEvent>

enum { METHOD_SPHERE, METHOD_CUBE };

int Tool::getOpposite(bool ignorePlanarBalls) const
{
    if (view->mirrorChanges)
    {
        const Ball &selection = view->doc->mesh.balls[view->selectedBall];
        bool isPlanar = (selection.center * Mesh::symmetryFlip - selection.center).lengthSquared() < 1.0e-8f;
        if (!ignorePlanarBalls && isPlanar) return view->selectedBall;
        return view->doc->mesh.getOppositeBall(view->selectedBall);
    }
    return -1;
}

int Tool::getSelection(int x, int y) const
{
    int detail = view->getDocument().mesh.getDetail();
    SelectionRecorder sel;
    view->camera3D();
    sel.enterSelectionMode(x, y);
    if (view->drawInterpolated) view->doc->mesh.drawInBetweenBalls();
    else view->doc->mesh.drawBones();
    for (int i = 0, count = view->doc->mesh.balls.count(); i < count; i++)
    {
        Ball &ball = view->doc->mesh.balls[i];
        sel.setObjectIndex(i);
        ball.draw(detail);
    }
    return sel.exitSelectionMode();
}

bool Tool::hitTestSelection(int x, int y, HitTest &result, int method) const
{
    if (view->selectedBall != -1)
    {
        // get selected ball
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        float radius = selection.maxRadius();

        // set up raytracer
        view->camera3D();
        Raytracer tracer;
        Vector3 origin = tracer.getEye();
        Vector3 ray = tracer.getRayForPixel(x, y);

        switch (method)
        {
        case METHOD_SPHERE:
            return Raytracer::hitTestSphere(selection.center, radius, origin, ray, result);

        case METHOD_CUBE:
            return Raytracer::hitTestCube(selection.center - radius, selection.center + radius, origin, ray, result);
        }
    }

    return false;
}

bool OrbitCameraTool::mousePressed(QMouseEvent *event)
{
    oldX = event->x();
    oldY = event->y();
    return true;
}

void OrbitCameraTool::mouseDragged(QMouseEvent *event)
{
    view->camera.theta += (event->x() - oldX) * 0.01f;
    view->camera.phi += (event->y() - oldY) * 0.01f;

    // keep theta in [0, 2pi] and phi in [-pi/2, pi/2]
    view->camera.theta -= floorf(view->camera.theta / M_2PI) * M_2PI;
    view->camera.phi = max(0.01f - M_PI / 2, min(M_PI / 2 - 0.01f, view->camera.phi));

    view->camera.update();
    oldX = event->x();
    oldY = event->y();
}

Vector3 MoveSelectionTool::getHit(QMouseEvent *event)
{
    // set up raytracer
    view->camera3D();
    Raytracer tracer;
    Vector3 origin = tracer.getEye();
    Vector3 ray = tracer.getRayForPixel(event->x(), event->y());

    // raytrace the plane, returning the original hit point if we don't hit the plane
    float t = (originalHit - origin).dot(planeNormal) / ray.dot(planeNormal);
    return (t > 0) ? origin + ray * t : originalHit;
}

bool MoveSelectionTool::mousePressed(QMouseEvent *event)
{
    HitTest result;
    if (hitTestSelection(event->x(), event->y(), result, METHOD_CUBE))
    {
        planeNormal = result.normal;
        originalHit = result.hit;
        originalCenter = view->doc->mesh.balls[view->selectedBall].center;
        return true;
    }

    return false;
}

void MoveSelectionTool::mouseDragged(QMouseEvent *event)
{
    if (view->selectedBall != -1)
    {
        // store the index of the symmetrically opposite ball
        int other = getOpposite(true);

        // move the selection
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        selection.center = originalCenter + getHit(event) - originalHit;

        // move the symmetrically opposite ball too
        if (other != -1) view->doc->mesh.balls[other].center = selection.center * Mesh::symmetryFlip;
    }
}

void MoveSelectionTool::mouseReleased(QMouseEvent *event)
{
    if (view->selectedBall != -1)
    {
        // store the index of the symmetrically opposite ball
        int other = getOpposite(true);

        // reset the ball and its opposite
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        selection.center = originalCenter;
        if (other != -1) view->doc->mesh.balls[other].center = selection.center * Mesh::symmetryFlip;

        // perform move if different
        Vector3 delta = getHit(event) - originalHit;
        if (delta.lengthSquared() > 0)
        {
            view->doc->getUndoStack().beginMacro("Move Ball");
            view->doc->moveBall(view->selectedBall, delta);
            if (other != -1) view->doc->moveBall(other, delta * Mesh::symmetryFlip);
            view->doc->getUndoStack().endMacro();
        }
    }
}

float ScaleSelectionTool::getScaleFactor(QMouseEvent *event)
{
    Raytracer tracer;
    Vector3 ray = tracer.getRayForPixel(event->x(), event->y());
    Ball &selection = view->doc->mesh.balls[view->selectedBall];
    float t = (selection.center - tracer.getEye()).dot(planeNormal) / ray.dot(planeNormal);
    Vector3 hit = tracer.getEye() + ray * t;
    return (hit - selection.center).length() / (originalHit - selection.center).length();
}

bool ScaleSelectionTool::mousePressed(QMouseEvent *event)
{
    HitTest result;
    if (hitTestSelection(event->x(), event->y(), result, METHOD_SPHERE))
    {
        Raytracer tracer;
        Vector3 ray = tracer.getRayForPixel(event->x(), event->y());
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        planeNormal = (tracer.getEye() - selection.center).unit();

        float t = (selection.center - tracer.getEye()).dot(planeNormal) / ray.dot(planeNormal);
        originalHit = tracer.getEye() + ray * t;
        originalX = selection.ex;
        originalY = selection.ey;
        originalZ = selection.ez;
    }
    return false;
}

void ScaleSelectionTool::mouseDragged(QMouseEvent *event)
{
    if (view->selectedBall != -1)
    {
        float scale = getScaleFactor(event);
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        selection.ex = originalX * scale;
        selection.ey = originalY * scale;
        selection.ez = originalZ * scale;
    }
}

void ScaleSelectionTool::mouseReleased(QMouseEvent *event)
{
    if (view->selectedBall != -1)
    {
        // reset the ball
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        selection.ex = originalX;
        selection.ey = originalY;
        selection.ez = originalZ;

        // perform scale if different
        float scale = getScaleFactor(event);
        if (fabsf(scale - 1) > 1.0e-6f)
        {
            view->doc->getUndoStack().beginMacro("Scale Ball");
            view->doc->scaleBall(view->selectedBall, originalX * scale, originalY * scale, originalZ * scale);
            view->doc->getUndoStack().endMacro();
        }
    }
}

bool SetAndMoveSelectionTool::mousePressed(QMouseEvent *event)
{
    view->selectedBall = getSelection(event->x(), event->y());
    if (view->selectedBall != -1)
    {
        MoveSelectionTool::mousePressed(event);
        return true;
    }

    return false;
}

bool SetAndScaleSelectionTool::mousePressed(QMouseEvent *event)
{
    view->selectedBall = getSelection(event->x(), event->y());
    if (view->selectedBall != -1)
    {
        ScaleSelectionTool::mousePressed(event);
        return true;
    }

    return false;
}

bool CreateBallTool::mousePressed(QMouseEvent *event)
{
    if (event->button() != Qt::RightButton)
        return false;

    // change the selection
    HitTest result;
    if (view->selectedBall == -1 || !hitTestSelection(event->x(), event->y(), result, METHOD_CUBE))
        view->selectedBall = getSelection(event->x(), event->y());

    // create the child ball
    if (view->selectedBall != -1)
    {
        // store the index of the symmetrically opposite ball
        int other = getOpposite(false);

        // create the new child
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        Ball child;
        child.center = selection.center;
        child.ex = selection.ex;
        child.ey = selection.ey;
        child.ez = selection.ez;
        child.parentIndex = view->selectedBall;

        // add the new child and prepare to move it around
        view->selectedBall = view->doc->mesh.balls.count();
        view->doc->getUndoStack().beginMacro("Create Ball");
        view->doc->addBall(child);
        if (other != -1)
        {
            // also add a symmetrically opposite child
            child.center *= Mesh::symmetryFlip;
            child.parentIndex = other;
            view->doc->addBall(child);
        }
        MoveSelectionTool::mousePressed(event);
        return true;
    }

    return false;
}

void CreateBallTool::mouseReleased(QMouseEvent *event)
{
    MoveSelectionTool::mouseReleased(event);
    view->doc->getUndoStack().endMacro();
}
