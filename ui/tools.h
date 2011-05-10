#ifndef TOOLS_H
#define TOOLS_H

#include "camera.h"
#include "raytracer.h"
#include "mesh.h"

class View;
class QMouseEvent;
class QWheelEvent;

class Tool : public QObject
{
protected:
    enum { METHOD_SPHERE, METHOD_CUBE };

    View *view;

    Ball &getSelectedBall();
    Ball &getOppositeBall();

    const Ball &getSelectedBall() const;
    const Ball &getOppositeBall() const;

    int getOppositeIndex(bool ignorePlanarBalls) const;
    int getSelection(int x, int y) const;
    bool hitTestSelection(int x, int y, HitTest &result, int method) const;

public:
    Tool(View *view) : view(view) {}

    virtual void drawDebug(int, int) {}
    virtual bool mousePressed(QMouseEvent *) = 0;
    virtual void mouseDragged(QMouseEvent *) {}
    virtual void mouseReleased(QMouseEvent *) {}
    virtual bool wheelEvent(QWheelEvent *) { return false; }
};

class OrbitCameraTool : public Tool
{
private:
    int originalX, originalY;

public:
    OrbitCameraTool(View *view) : Tool(view) {}

    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    bool wheelEvent(QWheelEvent *event);
};

class FirstPersonCameraTool : public Tool
{
private:
    enum { MODE_ROTATE, MODE_PAN, MODE_DOLLY };
    int originalX, originalY, mode;

public:
    FirstPersonCameraTool(View *view) : Tool(view) {}

    bool mousePressed(QMouseEvent *event);
    void mouseDragged(QMouseEvent *event);
    bool wheelEvent(QWheelEvent *event);
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
    int originalMouseY;
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
