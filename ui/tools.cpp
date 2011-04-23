#include "tools.h"
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
