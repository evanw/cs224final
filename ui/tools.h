#ifndef TOOLS_H
#define TOOLS_H

#include "camera.h"
#include "raytracer.h"

class View;
class QMouseEvent;

class Tool
{
protected:
    View *view;

    int getSelection(int x, int y);
    bool hitTestSelection(int x, int y, HitTest &result);

public:
    Tool(View *view) : view(view) {}

    virtual bool mousePressed(QMouseEvent *) = 0;
    virtual void mouseDragged(QMouseEvent *) {}
    virtual void mouseReleased(QMouseEvent *) {}
};

class OrbitCameraTool : public Tool
{
private:
    int oldX, oldY;

public:
    OrbitCameraTool(View *view) : Tool(view) {}

    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
};

class MoveSelectionTool : public Tool
{
private:
    Vector3 planeNormal;
    Vector3 originalHit;
    Vector3 originalCenter;

    Vector3 getHit(QMouseEvent *event);

public:
    MoveSelectionTool(View *view) : Tool(view) {}

    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

class SetSelectionTool : public MoveSelectionTool
{
public:
    SetSelectionTool(View *view) : MoveSelectionTool(view) {}

    bool mousePressed(QMouseEvent *event);
};

class CreateBallTool : public MoveSelectionTool
{
public:
    CreateBallTool(View *view) : MoveSelectionTool(view) {}

    bool mousePressed(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

#endif // TOOLS_H
