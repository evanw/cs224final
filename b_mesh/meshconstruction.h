#ifndef MESHCONSTRUCTION_H
#define MESHCONSTRUCTION_H

#include "mesh.h"
#include <iostream>

using namespace std;

class MeshConstruction
{
public:
    static void BMeshInit(Mesh *m);

private:
    static void sweep(Mesh *m, const Ball *b);
    static void stitch(Mesh *m);
    static Vector3 rotate(const Vector3 &p, const Vector3 &v, float radians);


};

#endif // MESHCONSTRUCTION_H
