#include "rawdocument.h"
#include "geometry.h"
#include <qgl.h>

void RawDocument::drawKeyBalls() const
{
    foreach (const Ellipsoid &ellipsoid, ellipsoids)
    {
        float matrix[16] = {
            ellipsoid.ex.x, ellipsoid.ex.y, ellipsoid.ex.z, 0,
            ellipsoid.ey.x, ellipsoid.ey.y, ellipsoid.ey.z, 0,
            ellipsoid.ez.x, ellipsoid.ez.y, ellipsoid.ez.z, 0,
            0, 0, 0, 1
        };
        glPushMatrix();
        glTranslatef(ellipsoid.center.x, ellipsoid.center.y, ellipsoid.center.z);
        glMultMatrixf(matrix);
        drawSphere();
        glPopMatrix();
    }
}
