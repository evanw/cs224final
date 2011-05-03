#include "camera.h"
#include <qgl.h>

void Camera::apply() const
{
    gluLookAt(eye.x, eye.y, eye.z, eye.x + dir.x, eye.y + dir.y, eye.z + dir.z, 0, 1, 0);
}

void OrbitCamera::update()
{
    dir = -Vector3::fromAngles(theta, phi);
    eye = center - dir * zoom;
}

void OrbitCamera::reset()
{
    theta = M_PI * 0.4;
    phi = M_PI * 0.1;
    zoom = 10;
    update();
}

void FirstPersonCamera::update()
{
    dir = -Vector3::fromAngles(theta, phi);
}

void FirstPersonCamera::reset()
{
    theta = M_PI * 0.4;
    phi = M_PI * 0.1;
    update();
    eye = -dir * 10;
}
