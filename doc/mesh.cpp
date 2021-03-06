#include "mesh.h"
#include "geometry.h"
#include <QSet>
#define GL_GLEXT_PROTOTYPES
#include <qgl.h>
#include <float.h>

#define COMPILE_TIME_ASSERT(pred) switch(0){case 0:case pred:;}
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

typedef QPair<int, int> Edge;

const Vector3 Mesh::symmetryFlip(-1, 1, 1);

inline void addEdge(QSet<Edge> &edges, int a, int b)
{
    edges += Edge(min(a, b), max(a, b));
}

bool Ball::isOppositeOf(const Ball &other) const
{
    const float epsilon = 1.0e-8f;
    return (center - other.center * Mesh::symmetryFlip).lengthSquared() < epsilon &&
            fabsf(minRadius() - other.minRadius()) < epsilon &&
            fabsf(maxRadius() - other.maxRadius()) < epsilon;
}

void Ball::draw(int detail) const
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
    drawSphere(detail);
    glPopMatrix();
}

void Vertex::draw() const
{
    glNormal3fv(normal.xyz);
    glVertex3fv(pos.xyz);
}

Mesh::Mesh()
#if ENABLE_GPU_UPLOAD
    : vertexBuffer(0), triangleIndexBuffer(0), lineIndexBuffer(0)
#endif
{
    subdivisionLevel = 0;
}

Mesh::~Mesh()
{
#if ENABLE_GPU_UPLOAD
    glDeleteBuffersARB(1, &vertexBuffer);
    glDeleteBuffersARB(1, &triangleIndexBuffer);
    glDeleteBuffersARB(1, &lineIndexBuffer);
#endif
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

void Mesh::uploadToGPU()
{
#if ENABLE_GPU_UPLOAD
    if (triangles.count() + quads.count() > 0)
    {
        QSet<Edge> edges;

        COMPILE_TIME_ASSERT(sizeof(Vector3) == sizeof(float) * 3);

        cachedVertices.clear();
        cachedTriangleIndices.clear();
        cachedLineIndices.clear();

        foreach (const Vertex &vertex, vertices)
        {
            cachedVertices += vertex.pos;
            cachedVertices += vertex.normal;
        }

        foreach (const Triangle &tri, triangles)
        {
            cachedTriangleIndices += tri.a.index;
            cachedTriangleIndices += tri.b.index;
            cachedTriangleIndices += tri.c.index;

            addEdge(edges, tri.a.index, tri.b.index);
            addEdge(edges, tri.b.index, tri.c.index);
            addEdge(edges, tri.c.index, tri.a.index);
        }

        foreach (const Quad &quad, quads)
        {
            cachedTriangleIndices += quad.a.index;
            cachedTriangleIndices += quad.b.index;
            cachedTriangleIndices += quad.c.index;

            cachedTriangleIndices += quad.a.index;
            cachedTriangleIndices += quad.c.index;
            cachedTriangleIndices += quad.d.index;

            addEdge(edges, quad.a.index, quad.b.index);
            addEdge(edges, quad.b.index, quad.c.index);
            addEdge(edges, quad.c.index, quad.d.index);
            addEdge(edges, quad.d.index, quad.a.index);
        }

        foreach (const Edge &edge, edges)
        {
            cachedLineIndices += edge.first;
            cachedLineIndices += edge.second;
        }

        if (!vertexBuffer) glGenBuffersARB(1, &vertexBuffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, cachedVertices.count() * sizeof(Vector3), &cachedVertices[0], GL_STATIC_DRAW_ARB);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        if (!triangleIndexBuffer) glGenBuffersARB(1, &triangleIndexBuffer);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, triangleIndexBuffer);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cachedTriangleIndices.count() * sizeof(int), &cachedTriangleIndices[0], GL_STATIC_DRAW_ARB);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        if (!lineIndexBuffer) glGenBuffersARB(1, &lineIndexBuffer);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, lineIndexBuffer);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cachedLineIndices.count() * sizeof(int), &cachedLineIndices[0], GL_STATIC_DRAW_ARB);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
#endif
}

void Mesh::drawKeyBalls(float alpha) const
{
    foreach (const Ball &ball, balls)
    {
        if (ball.parentIndex == -1)
            glColor4f(0.75, 0, 0, alpha);
        else
            glColor4f(0, 0.5, 1, alpha);
        ball.draw(BALL_DETAIL);
    }
}

void Mesh::drawInBetweenBalls() const
{
    foreach (const Ball &ball, balls)
    {
        // get the parent ball
        if (ball.parentIndex == -1) continue;
        const Ball &parent = balls[ball.parentIndex];

        // decide how many in-between balls to generate
        float totalRadius = ball.maxRadius() + parent.maxRadius();
        float edgeLength = (ball.center - parent.center).length();
        const int count = min(100, ceilf(edgeLength / totalRadius * 4));

        // generate in-between balls
        for (int i = 1; i < count; i++)
        {
            float percent = (float)i / (float)count;
            Ball tween;
            tween.center = Vector3::lerp(ball.center, parent.center, percent);
            tween.ex = Vector3::lerp(ball.ex, parent.ex, percent);
            tween.ey = Vector3::lerp(ball.ey, parent.ey, percent);
            tween.ez = Vector3::lerp(ball.ez, parent.ez, percent);
            tween.draw(BALL_DETAIL);
        }
    }
}

void Mesh::drawBones() const
{
    // draw bones as rotated and scaled cylinders
    foreach (const Ball &ball, balls)
    {
        if (ball.parentIndex == -1) continue;
        const Ball &parent = balls[ball.parentIndex];

        // calculate an appropriate radius based on the minimum ball size
        float radius = min(ball.minRadius(), parent.minRadius()) / 4;

        Vector3 delta = ball.center - parent.center;
        Vector2 angles = delta.toAngles();
        glPushMatrix();
        glTranslatef(parent.center.x, parent.center.y, parent.center.z);
        glRotatef(90 - angles.x * 180 / M_PI, 0, 1, 0);
        glRotatef(-angles.y * 180 / M_PI, 1, 0, 0);
        glScalef(radius, radius, delta.length());
        drawCylinder(BALL_DETAIL);
        glPopMatrix();
    }
}

int Mesh::getOppositeBall(int index) const
{
    const Ball &ball = balls[index];
    int oppositeIndex = -1;
    for (int i = 0; i < balls.count(); i++)
    {
        const Ball &opposite = balls[i];
        if (i != index && ball.isOppositeOf(opposite))
            oppositeIndex = i;
    }
    return oppositeIndex;
}

void Mesh::drawPoints() const
{
#if ENABLE_GPU_UPLOAD
    if (vertexBuffer && triangleIndexBuffer)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector3) * 2, BUFFER_OFFSET(0));
        glDrawArrays(GL_POINTS, 0, cachedVertices.count() / 2);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    else
#endif
    {
        glBegin(GL_POINTS);
        foreach (const Vertex &vertex, vertices)
            glVertex3fv(vertex.pos.xyz);
        glEnd();
    }
}

void Mesh::drawFill() const
{
#if ENABLE_GPU_UPLOAD
    if (vertexBuffer && triangleIndexBuffer)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector3) * 2, BUFFER_OFFSET(0));
        glNormalPointer(GL_FLOAT, sizeof(Vector3) * 2, BUFFER_OFFSET(sizeof(Vector3)));
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, triangleIndexBuffer);
        glDrawElements(GL_TRIANGLES, cachedTriangleIndices.count(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    else
#endif
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
#if ANIM_DEBUG
        glDisable(GL_DEPTH_TEST);
      glBegin(GL_LINES);
      foreach (const Vertex &vertex, vertices) {
          if (vertex.jointIndices[0] != -1) {
                glColor3f(vertex.jointWeights[0], 0, 0);
                glVertex3fv(vertex.pos.xyz);
                glVertex3fv(balls[vertex.jointIndices[0]].center.xyz);
          }
          if (vertex.jointIndices[1] != -1) {
              glColor3f(0, vertex.jointWeights[1], 0);
              glVertex3fv(vertex.pos.xyz);
              glVertex3fv(balls[vertex.jointIndices[1]].center.xyz);
          }
        }
      glEnd();
      glEnable(GL_DEPTH_TEST);
#endif
}

void Mesh::drawWireframe() const
{
#if ENABLE_GPU_UPLOAD
    if (vertexBuffer && lineIndexBuffer)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector3) * 2, BUFFER_OFFSET(0));
        glNormalPointer(GL_FLOAT, sizeof(Vector3) * 2, BUFFER_OFFSET(sizeof(Vector3)));
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, lineIndexBuffer);
        glDrawElements(GL_LINES, cachedLineIndices.count(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    else
#endif
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
}


Mesh * Mesh::copy() const
{
    Mesh *copy = new Mesh();
    copy->subdivisionLevel = subdivisionLevel;
    foreach (const Ball &ball, balls)
    {
        copy->balls += ball;
    }
    foreach (const Vertex &vert, vertices)
    {
        copy->vertices += vert;
    }
    foreach (const Triangle &tri, triangles)
    {
        copy->triangles += tri;
    }
    foreach (const Quad &quad, quads)
    {
        copy->quads += quad;
    }

    return copy;
}
