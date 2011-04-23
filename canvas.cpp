#include "canvas.h"

Canvas::Canvas(QWidget *parent) : QGLWidget(parent)
{
}

void Canvas::initializeGL()
{
}

void Canvas::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void Canvas::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
