#ifndef CURVATURE_H
#define CURVATURE_H

#include <QVector>
#include "mesh.h"

class Curvature
{
public:
    void computeCurvatures(const Mesh &mesh);
    void drawCurvatures(const Mesh &mesh);
    const QVector<float>& getMinCurvatures() { return minCurvatures; }
    const QVector<float>& getMaxCurvaturs() { return maxCurvatures; }
    const QVector<Vector3>& getMinDirections() { return minDirections; }
    const QVector<Vector3>& getMaxDirections() { return maxDirections; }

private:
    QVector<float> minCurvatures;
    QVector<float> maxCurvatures;
    QVector<Vector3> minDirections;
    QVector<Vector3> maxDirections;
};

#endif // CURVATURE_H
