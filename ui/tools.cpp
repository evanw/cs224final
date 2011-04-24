#include "tools.h"
#include "view.h"
#include "selectionrecorder.h"
#include <QMouseEvent>

int Tool::getSelection(int x, int y)
{
    SelectionRecorder sel;
    view->camera3D();
    sel.enterSelectionMode(x, y);
    if (view->drawInterpolated) view->doc->mesh.drawInBetweenBalls();
    else view->doc->mesh.drawBones();
    for (int i = 0, count = view->doc->mesh.balls.count(); i < count; i++)
    {
        Ball &ball = view->doc->mesh.balls[i];
        sel.setObjectIndex(i);
        ball.draw();
    }
    return sel.exitSelectionMode();
}

bool Tool::hitTestSelection(int x, int y, HitTest &result)
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

        // raytrace a cube around the selection
        return Raytracer::hitTestCube(selection.center - radius, selection.center + radius, origin, ray, result);
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

bool SetSelectionTool::mousePressed(QMouseEvent *event)
{
    if (view->mode == MODE_SKELETON && event->button() == Qt::LeftButton)
    {
        view->selectedBall = getSelection(event->x(), event->y());
        if (view->selectedBall != -1)
        {
            MoveSelectionTool::mousePressed(event);
            return true;
        }
    }

    return false;
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
    if (view->mode == MODE_SKELETON)
    {
        HitTest result;
        if (hitTestSelection(event->x(), event->y(), result))
        {
            planeNormal = result.normal;
            originalHit = result.hit;
            originalCenter = view->doc->mesh.balls[view->selectedBall].center;
            return true;
        }
    }

    return false;
}

void MoveSelectionTool::mouseDragged(QMouseEvent *event)
{
    if (view->selectedBall != -1)
    {
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        selection.center = originalCenter + getHit(event) - originalHit;
    }
}

void MoveSelectionTool::mouseReleased(QMouseEvent *event)
{
    if (view->selectedBall != -1)
    {
        Vector3 delta = getHit(event) - originalHit;
        if (delta.lengthSquared() > 0)
        {
            Ball &selection = view->doc->mesh.balls[view->selectedBall];
            selection.center = originalCenter;
            view->doc->getUndoStack().beginMacro("Move Ball");
            view->doc->moveBall(view->selectedBall, delta);
            view->doc->getUndoStack().endMacro();
        }
    }
}

bool CreateBallTool::mousePressed(QMouseEvent *event)
{
    if (view->mode == MODE_SKELETON && event->button() == Qt::RightButton)
    {
        // change the selection
        HitTest result;
        if (view->selectedBall == -1 || !hitTestSelection(event->x(), event->y(), result))
            view->selectedBall = getSelection(event->x(), event->y());

        // create the child ball
        if (view->selectedBall != -1)
        {
            Ball &selection = view->doc->mesh.balls[view->selectedBall];
            Ball child;
            child.center = selection.center;
            child.ex = selection.ex;
            child.ey = selection.ey;
            child.ez = selection.ez;
            child.parentIndex = view->selectedBall;
            view->selectedBall = view->doc->mesh.balls.count();

            view->doc->getUndoStack().beginMacro("Create Ball");
            view->doc->addBall(child);
            MoveSelectionTool::mousePressed(event);
            return true;
        }
    }

    return false;
}

void CreateBallTool::mouseReleased(QMouseEvent *event)
{
    MoveSelectionTool::mouseReleased(event);
    view->doc->getUndoStack().endMacro();
}
