#include "mesh.h"
#include "geometry.h"
#include <qgl.h>

void Ball::draw() const
{
    float matrix[16] = {
        ex.x, ex.y, ex.z, 0,
        ey.x, ey.y, ey.z, 0,
        ez.x, ez.y, ez.z, 0,
        0, 0, 0, 1
    };
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    glMultMatrixf(matrix);
    drawSphere();
    glPopMatrix();
}

void Mesh::drawKeyBalls(int boneType) const
{
    // draw key balls
    glColor3f(0, 0.5, 1);
    foreach (const Ball &ball, balls)
        ball.draw();

    switch (boneType)
    {
    case BONE_TYPE_INTERPOLATE:
        // draw in between balls
        glColor3f(0.75, 0.75, 0.75);
        foreach (const Ball &ball, balls)
        {
            if (ball.parentIndex == -1) continue;
            const Ball &parent = balls[ball.parentIndex];

            for (int i = 1, count = 7; i < count; i++)
            {
                float percent = (float)i / (float)count;
                Ball tween;
                tween.center = Vector3::lerp(ball.center, parent.center, percent);
                tween.ex = Vector3::lerp(ball.ex, parent.ex, percent);
                tween.ey = Vector3::lerp(ball.ey, parent.ey, percent);
                tween.ez = Vector3::lerp(ball.ez, parent.ez, percent);
                tween.draw();
            }
        }
        break;

    case BONE_TYPE_CYLINDER:
        // draw bones
        glColor3f(0.75, 0.75, 0.75);
        foreach (const Ball &ball, balls)
        {
            if (ball.parentIndex == -1) continue;
            const Ball &parent = balls[ball.parentIndex];

            const float radius = 0.05f;
            Vector3 delta = ball.center - parent.center;
            Vector2 angles = delta.toAngles();
            glPushMatrix();
            glTranslatef(parent.center.x, parent.center.y, parent.center.z);
            glRotatef(90 - angles.x * 180 / M_PI, 0, 1, 0);
            glRotatef(-angles.y * 180 / M_PI, 1, 0, 0);
            glScalef(radius, radius, delta.length());
            drawCylinder();
            glPopMatrix();
        }
        break;
    }
}
