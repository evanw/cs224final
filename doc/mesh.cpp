#include "mesh.h"
#include "geometry.h"
#include <qgl.h>
#include <float.h>

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

void Vertex::draw() const
{
    glNormal3fv(normal.xyz);
    glVertex3fv(pos.xyz);
}

void Mesh::updateChildIndices()
{
    for (int i = 0; i < balls.count(); i++)
    {
        Ball &ball = balls[i];
        ball.childrenIndices.clear();
    }

    for (int i = 0; i < balls.count(); i++)
    {
        Ball &ball = balls[i];
        if (ball.parentIndex != -1)
            balls[ball.parentIndex].childrenIndices += i;
    }
}

void Mesh::updateNormals()
{
    for (int i = 0; i < vertices.count(); i++)
    {
        Vertex &vertex = vertices[i];
        vertex.normal = Vector3();
    }

    for (int i = 0; i < triangles.count(); i++)
    {
        Triangle &tri = triangles[i];
        Vertex &a = vertices[tri.a.index];
        Vertex &b = vertices[tri.b.index];
        Vertex &c = vertices[tri.c.index];
        Vector3 normal = (b.pos - a.pos).cross(c.pos - a.pos).unit();
        a.normal += normal;
        b.normal += normal;
        c.normal += normal;
    }

    for (int i = 0; i < quads.count(); i++)
    {
        Quad &quad = quads[i];
        Vertex &a = vertices[quad.a.index];
        Vertex &b = vertices[quad.b.index];
        Vertex &c = vertices[quad.c.index];
        Vertex &d = vertices[quad.d.index];
        Vector3 normal = (
                (b.pos - a.pos).cross(d.pos - a.pos) +
                (c.pos - b.pos).cross(a.pos - b.pos) +
                (d.pos - c.pos).cross(b.pos - c.pos) +
                (a.pos - d.pos).cross(c.pos - d.pos)
            ).unit();
        a.normal += normal;
        b.normal += normal;
        c.normal += normal;
        d.normal += normal;
    }

    for (int i = 0; i < vertices.count(); i++)
    {
        Vertex &vertex = vertices[i];
        vertex.normal.normalize();
    }
}

void Mesh::drawKeyBalls(float alpha) const
{
    foreach (const Ball &ball, balls)
    {
        if (ball.parentIndex == -1)
            glColor4f(0.75, 0, 0, alpha);
        else
            glColor4f(0, 0.5, 1, alpha);
        ball.draw();
    }
}

void Mesh::drawInBetweenBalls() const
{
    foreach (const Ball &ball, balls)
    {
        if (ball.parentIndex == -1) continue;
        const Ball &parent = balls[ball.parentIndex];

        float totalRadius = ball.maxRadius() + parent.maxRadius();
        float edgeLength = (ball.center - parent.center).length();
        const int count = min(100, ceilf(edgeLength / totalRadius * 4));

        for (int i = 1; i < count; i++)
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
}

void Mesh::drawBones() const
{
    // calculate an appropriate radius based on the minimum ball size
    float radius = FLT_MAX;
    foreach (const Ball &ball, balls)
        radius = min(radius, ball.minRadius() / 4);

    // draw bones as rotated and scaled cylinders
    foreach (const Ball &ball, balls)
    {
        if (ball.parentIndex == -1) continue;
        const Ball &parent = balls[ball.parentIndex];

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
}

void Mesh::drawFill() const
{
    glBegin(GL_TRIANGLES);
    foreach (const Triangle &tri, triangles)
    {
        vertices[tri.a.index].draw();
        vertices[tri.b.index].draw();
        vertices[tri.c.index].draw();
    }
    glEnd();
    glBegin(GL_QUADS);
    foreach (const Quad &quad, quads)
    {
        vertices[quad.a.index].draw();
        vertices[quad.b.index].draw();
        vertices[quad.c.index].draw();
        vertices[quad.d.index].draw();
    }
    glEnd();
}

void Mesh::drawWireframe() const
{
    foreach (const Triangle &tri, triangles)
    {
        glBegin(GL_LINE_LOOP);
        vertices[tri.a.index].draw();
        vertices[tri.b.index].draw();
        vertices[tri.c.index].draw();
        glEnd();
    }
    foreach (const Quad &quad, quads)
    {
        glBegin(GL_LINE_LOOP);
        vertices[quad.a.index].draw();
        vertices[quad.b.index].draw();
        vertices[quad.c.index].draw();
        vertices[quad.d.index].draw();
        glEnd();
    }
}
