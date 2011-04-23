#include "selectionrecorder.h"
#include <qgl.h>

SelectionRecorder::SelectionRecorder()
{
}

SelectionRecorder::~SelectionRecorder()
{
}

void SelectionRecorder::enterSelectionMode(int x, int y)
{
    // Pre-multiply the projection matrix with a pick matrix, which expands
    // the viewport to fill the 1x1 square under the mouse pointer at (x, y).
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix((double)x, (double)(viewport[3] - y - 1), 1, 1, viewport);
    glMultMatrixd(projectionMatrix);
    glMatrixMode(GL_MODELVIEW);

    // restrict the viewport to [0,1]x[0,1]
    glEnable(GL_SCISSOR_BOX);
    glScissor(0, 0, 1, 1);
    glViewport(0, 0, 1, 1);

    // set an invalid object index
    double clearColor[4];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    depthTest = glIsEnabled(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
}

void SelectionRecorder::setObjectIndex(int index)
{
    glColor3ub(index & 0xFF, (index >> 8) & 0xFF, (index >> 16) & 0xFF);
}

int SelectionRecorder::exitSelectionMode()
{
    // read the id
    unsigned char rgb[3];
    glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, rgb);

    // reset opengl
    if (!depthTest) glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_BOX);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projectionMatrix);
    glMatrixMode(GL_MODELVIEW);

    int index = rgb[0] | (rgb[1] << 8) | (rgb[2] << 16);
    return index == 0xFFFFFF ? -1 : index;
}
