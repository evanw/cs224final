#ifndef CONVEXHULLSOLVER_H
#define CONVEXHULLSOLVER_H

#include <QList>
#include "vector.h"
#include "mesh.h"

namespace ConvexHullSolver
{
    bool run(Mesh &mesh);
    //         QList<Vector3>& hullPoints);
}

#endif // CONVEXHULLSOLVER_H
