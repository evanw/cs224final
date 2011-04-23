#include "tools.h"
#include "view.h"
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
    // select the ball under the mouse, or -1 for no selection
    SelectionRecorder sel;
    view->camera3D();
    sel.enterSelectionMode(event->x(), event->y());
    for (int i = 0, count = view->doc->mesh.balls.count(); i < count; i++)
    {
        Ball &ball = view->doc->mesh.balls[i];
        sel.setObjectIndex(i);
        ball.draw();
    }
    view->selectedBall = sel.exitSelectionMode();
    return view->selectedBall != -1;
}

bool MoveSelectionTool::mousePressed(QMouseEvent *event)
{
    return false;
}

void MoveSelectionTool::mouseDragged(QMouseEvent *event)
{
}

void MoveSelectionTool::mouseReleased(QMouseEvent *event)
{
}
