#include "raytracer.h"
#include <qgl.h>

#include <iostream>
using namespace std;

void HitTest::mergeWith(const HitTest &other)
{
    if (other.t > 0 && other.t < t)
    {
        t = other.t;
        hit = other.hit;
        normal = other.normal;
    }
}

static Vector3 unProject(float winX, float winY, double *modelview, double *projection, int *viewport)
{
    double objX, objY, objZ;
    gluUnProject(winX, winY, 1, modelview, projection, viewport, &objX, &objY, &objZ);
    return Vector3(objX, objY, objZ);
}

Raytracer::Raytracer()
{
    // read camera information from opengl
    double modelview[16];
    double projection[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    // reconstruct the eye position
    Vector3 xaxis(modelview[0], modelview[1], modelview[2]);
    Vector3 yaxis(modelview[4], modelview[5], modelview[6]);
    Vector3 zaxis(modelview[8], modelview[9], modelview[10]);
    Vector3 offset(modelview[12], modelview[13], modelview[14]);
    eye = -Vector3(offset.dot(xaxis), offset.dot(yaxis), offset.dot(zaxis));

    // generate the four corner rays
    int xmin = viewport[0];
    int ymin = viewport[1];
    int xmax = xmin + viewport[2] - 1;
    int ymax = ymin + viewport[3] - 1;
    ray00 = unProject(xmin, ymin, modelview, projection, viewport) - eye;
    ray10 = unProject(xmax, ymin, modelview, projection, viewport) - eye;
    ray01 = unProject(xmin, ymax, modelview, projection, viewport) - eye;
    ray11 = unProject(xmax, ymax, modelview, projection, viewport) - eye;
}

Vector3 Raytracer::getRayForPixel(int x, int y) const
{
    float fx = (float)(x - viewport[0]) / (float)viewport[2];
    float fy = 1 - (float)(y - viewport[1]) / (float)viewport[3];
    Vector3 ray0 = Vector3::lerp(ray00, ray10, fx);
    Vector3 ray1 = Vector3::lerp(ray01, ray11, fx);
    return Vector3::lerp(ray0, ray1, fy).unit();
}

bool Raytracer::hitTestCube(const Vector3 &cubeMin, const Vector3 &cubeMax, const Vector3 &origin, const Vector3 &ray, HitTest &result)
{
    // This uses the slab intersection method
    Vector3 tMin = (cubeMin - origin) / ray;
    Vector3 tMax = (cubeMax - origin) / ray;
    Vector3 t1 = Vector3::min(tMin, tMax);
    Vector3 t2 = Vector3::max(tMin, tMax);
    float tNear = t1.max();
    float tFar = t2.min();

    if (tNear > 0 && tNear < tFar)
    {
        const float epsilon = 1.0e-6;
        result.hit = origin + ray * tNear;
        if (result.hit.x < cubeMin.x + epsilon) result.normal = Vector3(-1, 0, 0);
        else if (result.hit.y < cubeMin.y + epsilon) result.normal = Vector3(0, -1, 0);
        else if (result.hit.z < cubeMin.z + epsilon) result.normal = Vector3(0, 0, -1);
        else if (result.hit.x > cubeMax.x - epsilon) result.normal = Vector3(1, 0, 0);
        else if (result.hit.y > cubeMax.y - epsilon) result.normal = Vector3(0, 1, 0);
        else result.normal = Vector3(0, 0, 1);
        result.t = tNear;
        return true;
    }

    return false;
}

bool Raytracer::hitTestSphere(const Vector3 &center, float radius, const Vector3 &origin, const Vector3 &ray, HitTest &result)
{
    Vector3 offset = origin - center;
    float a = ray.lengthSquared();
    float b = 2 * ray.dot(offset);
    float c = offset.lengthSquared() - radius * radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant > 0)
    {
        result.t = (-b - sqrtf(discriminant)) / (2 * a);
        result.hit = origin + ray * result.t;
        result.normal = (center - result.hit) / radius;
        return true;
    }

    return false;
}

bool Raytracer::hitTestTriangle(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &origin, const Vector3 &ray, HitTest &result)
{
    Vector3 ab = b - a;
    Vector3 ac = c - a;
    result.normal = ab.cross(ac).unit();
    result.t = result.normal.dot(a - origin) / result.normal.dot(ray);

    if (result.t > 0)
    {
        result.hit = origin + ray * result.t;
        Vector3 toHit = result.hit - a;
        float dot00 = ac.lengthSquared();
        float dot01 = ac.dot(ab);
        float dot02 = ac.dot(toHit);
        float dot11 = ab.lengthSquared();
        float dot12 = ab.dot(toHit);
        float divide = dot00 * dot11 - dot01 * dot01;
        float u = (dot11 * dot02 - dot01 * dot12) / divide;
        float v = (dot00 * dot12 - dot01 * dot02) / divide;
        return (u >= 0 && v >= 0 && u + v <= 1);
    }

    return false;
}
