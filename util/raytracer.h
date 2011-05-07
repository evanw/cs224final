#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "vector.h"
#include <float.h>

struct HitTest
{
    float t;
    Vector3 hit;
    Vector3 normal;

    HitTest() : t(FLT_MAX) {}

    void mergeWith(const HitTest &other);
};

class Raytracer
{
private:
    int viewport[4];
    Vector3 eye, ray00, ray10, ray01, ray11;

public:
    Raytracer();

    Vector3 getEye() const { return eye; }
    Vector3 getRayForPixel(int x, int y) const;
    static bool hitTestCube(const Vector3 &min, const Vector3 &max, const Vector3 &origin, const Vector3 &ray, HitTest &result);
    static bool hitTestSphere(const Vector3 &center, float radius, const Vector3 &origin, const Vector3 &ray, HitTest &result);
    static bool hitTestTriangle(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &origin, const Vector3 &ray, HitTest &result);
};

#endif // RAYTRACER_H
