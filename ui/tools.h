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

    int getOpposite(bool ignorePlanarBalls) const;
    int getSelection(int x, int y) const;
    bool hitTestSelection(int x, int y, HitTest &result, int method) const;

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

class ScaleSelectionTool : public Tool
{
private:
    Vector3 planeNormal;
    Vector3 originalHit;
    Vector3 originalX;
    Vector3 originalY;
    Vector3 originalZ;

    float getScaleFactor(QMouseEvent *event);

public:
    ScaleSelectionTool(View *view) : Tool(view) {}

    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
};

class SetAndMoveSelectionTool : public MoveSelectionTool
{
public:
    SetAndMoveSelectionTool(View *view) : MoveSelectionTool(view) {}

    bool mousePressed(QMouseEvent *event);
};

class SetAndScaleSelectionTool : public ScaleSelectionTool
{
public:
    SetAndScaleSelectionTool(View *view) : ScaleSelectionTool(view) {}

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
