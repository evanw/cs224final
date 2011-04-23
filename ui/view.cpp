#include "view.h"
#include "geometry.h"

View::View(QWidget *parent) : QGLWidget(parent), currentTool(NULL)
{
    Ellipsoid ellipsoid;
    doc.raw.ellipsoids += ellipsoid;
}

void View::initializeGL()
{
    camera.theta = M_PI / 4;
    camera.phi = M_PI / 6;
    camera.zoom = 5;
    camera.update();

    tools += new OrbitCameraTool(camera);

    float ambient[4] = { 0.4, 0.4, 0.4, 0 };
    float diffuse[4] = { 0.6, 0.6, 0.6, 0 };
    float specular[4] = { 1, 1, 1, 0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

    glEnable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.75, 0.75, 0.75, 0);
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

    float position[4] = { 0, 1, 0, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glColor3f(0, 0.5, 1);
    glEnable(GL_LIGHTING);
    doc.raw.drawKeyBalls();
    glDisable(GL_LIGHTING);

    drawGrid();
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

void View::drawGrid() const
{
    const int size = 10;
    glColor3f(0, 0, 0);
    glDepthMask(GL_FALSE);
    glLineWidth(0.5);
    glEnable(GL_BLEND);
    glBegin(GL_LINES);
    for (int x = -size; x <= size; x++)
    {
        for (int z = -size; z <= size; z++)
        {
            if (x != size)
            {
                glVertex3i(x, 0, z);
                glVertex3i(x + 1, 0, z);
            }
            if (z != size)
            {
                glVertex3i(x, 0, z);
                glVertex3i(x, 0, z + 1);
            }
        }
    }
    glEnd();
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
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
    gluPerspective(45, (float)width() / (float)height(), 0.01, 100.0);
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
