#ifndef CANVAS_H
#define CANVAS_H

#include <QGLWidget>

class Canvas : public QGLWidget
{
public:
    Canvas(QWidget *parent);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
};

#endif // CANVAS_H
