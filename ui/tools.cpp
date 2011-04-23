#include "tools.h"
#include "view.h"
#include "raytracer.h"
#include "selectionrecorder.h"
#include <QMouseEvent>

bool OrbitCameraTool::mousePressed(QMouseEvent *event)
{
    oldX = event->x();
    oldY = event->y();
    return true;
}

void OrbitCameraTool::mouseDragged(QMouseEvent *event)
{
    camera.theta += (event->x() - oldX) * 0.01f;
    camera.phi += (event->y() - oldY) * 0.01f;

    // keep theta in [0, 2pi] and phi in [-pi/2, pi/2]
    camera.theta -= floorf(camera.theta / M_2PI) * M_2PI;
    camera.phi = max(0.01f - M_PI / 2, min(M_PI / 2 - 0.01f, camera.phi));

    camera.update();
    oldX = event->x();
    oldY = event->y();
}

bool SetSelectionTool::mousePressed(QMouseEvent *event)
{
    if (view->mode == MODE_SKELETON)
    {
        // select the ball under the mouse, or -1 for no selection
        SelectionRecorder sel;
        view->camera3D();
        sel.enterSelectionMode(event->x(), event->y());
        view->doc->mesh.drawInBetweenBalls();
        for (int i = 0, count = view->doc->mesh.balls.count(); i < count; i++)
        {
            Ball &ball = view->doc->mesh.balls[i];
            sel.setObjectIndex(i);
            ball.draw();
        }
        view->selectedBall = sel.exitSelectionMode();
        return view->selectedBall != -1;
    }

    return false;
}

enum { XNEG, XPOS, YNEG, YPOS, ZNEG, ZPOS };

#include <iostream>
using namespace std;

bool MoveSelectionTool::mousePressed(QMouseEvent *event)
{
    if (view->mode == MODE_SKELETON && view->selectedBall != -1)
    {
        // get selected ball
        Ball &selection = view->doc->mesh.balls[view->selectedBall];
        float radius = selection.maxRadius();

        // set up raytracer
        view->camera3D();
        Raytracer tracer;
        Vector3 origin = tracer.getEye();
        Vector3 ray = tracer.getRayForPixel(event->x(), event->y());

        // raytrace a cube around the selection
        HitTest result;
        if (Raytracer::hitTestCube(selection.center - radius, selection.center + radius, origin, ray, result))
        {
            return true;
        }
    }

    return false;
}

void MoveSelectionTool::mouseDragged(QMouseEvent *event)
{
}

void MoveSelectionTool::mouseReleased(QMouseEvent *event)
{
}
