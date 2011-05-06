#ifndef GEOM_H
#define GEOM_H

#include "vector.h"

void drawCube();
void drawSphere(int detail);
void drawCylinder(int detail);
void drawWireCube();
void drawWireCube(const Vector3 &minCorner, const Vector3 &maxCorner);
void drawWireDisk();
void drawMoveCursor();
void drawScaleCursor();

#endif // GEOM_H
