#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

struct Camera
{
    Vector3 eye;
    Vector3 dir;

    void apply() const;
};

struct OrbitCamera : public Camera
{
    Vector3 center;
    float theta;
    float phi;
    float zoom;

    void update();
    void reset();
};

struct FirstPersonCamera : public Camera
{
    float theta;
    float phi;

    void update();
    void reset();
};

#endif // CAMERA_H
