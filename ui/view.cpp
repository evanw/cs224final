#include "view.h"
#include "geometry.h"
#include <QWheelEvent>

View::View(QWidget *parent) : QGLWidget(parent), selectedBall(-1), currentTool(NULL)
{
    selectedBall = 1;
    doc.raw.balls += Ball(Vector3(0, 1.25, 0), 0.5);
    doc.raw.balls += Ball(Vector3(-0.75, 0.75, 0.25), 0.25, 0);
    doc.raw.balls += Ball(Vector3(0.75, 0.75, 0.25), 0.25, 0);
    doc.raw.balls += Ball(Vector3(-0.5, 0, 0), 0.1, 1);
    doc.raw.balls += Ball(Vector3(0.5, 0, 0), 0.1, 2);
    doc.raw.balls[0].ex *= 0.25;
}

void View::initializeGL()
{
    camera.theta = M_PI * 0.4;
    camera.phi = M_PI * 0.1;
    camera.zoom = 10;
    camera.update();

    tools += new OrbitCameraTool(camera);

    // opengl lighting
    float ambient0[4] = { 0.4, 0.4, 0.4, 0 };
    float diffuse0[4] = { 0.6, 0.6, 0.6, 0 };
    float diffuse1[4] = { -0.2, -0.2, -0.2, 0 };
    float specular[4] = { 1, 1, 1, 0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

    // other opengl state
    glLineWidth(0.5);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.875, 0.875, 0.875, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void View::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void View::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera3D();

    float position0[4] = { 0, 1, 0, 0 };
    float position1[4] = { 0, -1, 0, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, position0);
    glLightfv(GL_LIGHT1, GL_POSITION, position1);

    // draw model
    glEnable(GL_LIGHTING);
    doc.raw.drawKeyBalls(BONE_TYPE_INTERPOLATE);
    glDisable(GL_LIGHTING);

    // enable line drawing
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    // draw box around selected ball
    if (selectedBall != -1)
    {
        Ball &ball = doc.raw.balls[selectedBall];
        float radius = ball.maxRadius();
        glPushMatrix();
        glTranslatef(ball.center.x, ball.center.y, ball.center.z);
        glScalef(radius, radius, radius);
        glDisable(GL_DEPTH_TEST);
        glColor4f(0, 0, 0, 0.25f);
        drawWireCube();
        glEnable(GL_DEPTH_TEST);
        glColor3f(0, 0, 0);
        drawWireCube();
        glPopMatrix();
    }

    // draw the ground plane
    drawGrid();

    // disable line drawing
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void View::mousePressEvent(QMouseEvent *event)
{
    // old mouse up
    if (currentTool)
    {
        currentTool->mouseReleased(event);
        currentTool = NULL;
    }

    // new mouse down, use first tool to accept mouse event
    foreach (Tool *tool, tools)
    {
        if (tool->mousePressed(event))
        {
            currentTool = tool;
            break;
        }
    }

    updateGL();
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    if (currentTool)
    {
        currentTool->mouseDragged(event);
        updateGL();
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentTool)
    {
        currentTool->mouseReleased(event);
        updateGL();
    }
}

void View::wheelEvent(QWheelEvent *event)
{
    camera.zoom *= powf(0.999f, event->delta());
    camera.update();
    updateGL();
}

void View::drawGrid() const
{
    const int size = 10;
    glBegin(GL_LINES);
    for (int x = -size; x <= size; x++)
    {
        for (int z = -size; z <= size; z++)
        {
            if (x != size)
            {
                glColor4f(0, 0, 0, z == 0 ? 1 : 0.25);
                glVertex3i(x, 0, z);
                glVertex3i(x + 1, 0, z);
            }
            if (z != size)
            {
                glColor4f(0, 0, 0, x == 0 ? 1 : 0.25);
                glVertex3i(x, 0, z);
                glVertex3i(x, 0, z + 1);
            }
        }
    }
    glEnd();
}

void View::camera2D() const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void View::camera3D() const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)width() / (float)height(), 0.01, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.apply();
}

void View::undo()
{
    doc.undo();
    updateGL();
}

void View::redo()
{
    doc.redo();
    updateGL();
}
