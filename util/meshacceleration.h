#ifndef MESHACCELERATION_H
#define MESHACCELERATION_H

#include "metamesh.h"
#include "raytracer.h"
#include <QSet>

class AccelerationDataStructure
{
protected:
    MetaMesh &mesh;

public:
    AccelerationDataStructure(MetaMesh &mesh) : mesh(mesh) {}

    virtual void drawDebug() = 0;
    virtual void updateVertices(const QVector<MetaVertex *> &vertices) = 0;
    virtual bool hitTest(const Vector3 &origin, const Vector3 &ray, HitTest &result) const = 0;
    virtual void getVerticesInSphere(const Vector3 &center, float radius, QVector<MetaVertex *> &vertices) const = 0;
};

/**
 * A voxel grid that is fitted around a mesh before sculpting begins to
 * speed up queries. This must be kept up to date as the mesh is modified.
 *
 * Note: this completely ignores triangles because we only generate quads.
 */
class VoxelGrid : public AccelerationDataStructure
{
private:
    class Voxel
    {
    public:
        QSet<MetaVertex *> vertices;
        QSet<Quad *> quads;
    };

    Vector3 minCorner, maxCorner;
    int countX, countY, countZ;

    /**
     * Has countX * countY * countZ + 1 voxels (everything that doesn't
     * fit into the grid goes in the last one).
     */
    QVector<Voxel> voxels;

    /**
     * Convert world space to grid space.
     */
    Vector3 convertToGrid(const Vector3 &pos) const;

    /**
     * Convert grid space to world space.
     */
    Vector3 convertFromGrid(const Vector3 &pos) const;

    /**
     * Returns an index into voxels (will return countX * countY * countZ
     * if pos is outside the grid).
     */
    int getVoxelForPos(const Vector3 &pos) const;

    /**
     * Convert grid space into an index into voxels (will return countX *
     * countY * countZ if pos is outside the grid).
     */
    int getIndex(int x, int y, int z) const;

    /**
     * Convert grid space into an index into voxels without bounds checking.
     */
    int getInternalIndex(int x, int y, int z) const;

public:
    VoxelGrid(MetaMesh &mesh, float spacing);

    void drawDebug();
    void updateVertices(const QVector<MetaVertex *> &vertices);
    bool hitTest(const Vector3 &origin, const Vector3 &ray, HitTest &result) const;
    void getVerticesInSphere(const Vector3 &center, float radius, QVector<MetaVertex *> &vertices) const;
};

#endif // MESHACCELERATION_H
