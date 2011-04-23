#include "geometry.h"
#include <qgl.h>

GLUquadric *quadric = NULL;
const float cubeVertices[8 * 3] =
{
    -1, -1, -1,
    +1, -1, -1,
    -1, +1, -1,
    +1, +1, -1,
    -1, -1, +1,
    +1, -1, +1,
    -1, +1, +1,
    +1, +1, +1,
};
const int cubeFaces[] =
{
    0, 2, 3, 1,
    4, 5, 7, 6,
    0, 1, 5, 4,
    2, 6, 7, 3,
    0, 4, 6, 2,
    1, 3, 7, 5,
};
const int cubeLines[] =
{
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 2, 1, 3, 4, 6, 5, 7,
    0, 4, 1, 5, 2, 6, 3, 7,
};

static void initQuadric()
{
    if (!quadric)
        quadric = gluNewQuadric();
}

void drawCube()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
    glDrawElements(GL_QUADS, sizeof(cubeFaces) / sizeof(*cubeFaces), GL_UNSIGNED_INT, cubeFaces);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void drawSphere()
{
    initQuadric();
    gluSphere(quadric, 1, 64, 32);
}

void drawWireCube()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
    glDrawElements(GL_LINES, sizeof(cubeLines) / sizeof(*cubeLines), GL_UNSIGNED_INT, cubeLines);
    glDisableClientState(GL_VERTEX_ARRAY);
}
