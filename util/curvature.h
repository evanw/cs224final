#ifndef CURVATURE_H
#define CURVATURE_H

#include <QVector>
#include "mesh.h"

class Curvature
{
public:
    Curvature();

    void computeCurvatures(const Mesh &mesh);

private:
    QVector<float> minCurvatures;
    QVector<float> maxCurvatures;
    QVector<Vector3> minDirections;
    QVector<Vector3> maxDirections;
};

#endif // CURVATURE_H
