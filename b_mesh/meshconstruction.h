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
    static Quad sweep(Mesh *m, const Ball *b);
    static void stitch(Mesh *m);
    //rotate point p around axis v centered at c by radians
    static Vector3 rotate(const Vector3 &p, const Vector3 &v, const Vector3 &c, float radians);


};

#endif // MESHCONSTRUCTION_H
