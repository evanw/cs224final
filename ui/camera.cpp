#include "camera.h"
#include <qgl.h>

void Camera::apply() const
{
    gluLookAt(eye.x, eye.y, eye.z, eye.x + dir.x, eye.y + dir.y, eye.z + dir.z, 0, 1, 0);
}

void OrbitCamera::update()
{
    dir = -Vector3::fromAngle(theta, phi);
    eye = center - dir * zoom;
}
