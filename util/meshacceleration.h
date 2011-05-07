#ifndef MESHACCELERATION_H
#define MESHACCELERATION_H

#include "metamesh.h"
#include "raytracer.h"
#include <QSet>

/**
 * An abstract class that supports accelerated raytracing and neighbor
 * queries on a partially dynamic mesh.
 */
class AccelerationDataStructure
{
protected:
    MetaMesh &mesh;

public:
    AccelerationDataStructure(MetaMesh &mesh) : mesh(mesh) {}

    /**
     * Optionally draw helpful debugging information.
     */
    virtual void drawDebug() = 0;

    /**
     * Returns true if the ray hits a quad, in which case result will
     * contain the hit test result. Result may be modified even if this
     * method returns false.
     */
    virtual bool hitTest(const Vector3 &origin, const Vector3 &ray, HitTest &result) const = 0;

    /**
     * Fills vertices with all vertices that may be in the axis-aligned
     * bounding box. This method may also return vertices that are
     * nowhere near the bounding box.
     */
    virtual void getVerticesInAABB(const Vector3 &minCoord, const Vector3 &maxCoord, QSet<MetaVertex *> &vertices) = 0;

    /**
     * Update the acceleration data structure to reflect the new positions
     * of all vertices in changedVertices.
     */
    virtual void updateVertices(const QSet<MetaVertex *> &changedVertices) = 0;
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
     * Remember the voxels we added each quad to so we can remove them later.
     */
    QHash<Quad *, QSet<int> > voxelsForQuad;

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

    /**
     * Hit test the two triangles that make up the quad and modify result if
     * either of those hit tests were closer than what is already in result.
     */
    void hitTestHelper(Quad *quad, const Vector3 &origin, const Vector3 &ray, HitTest &result) const;

    /**
     * Place quad in all voxels in or touching the AABB of all the vertices.
     */
    void addQuadToVoxels(Quad &quad);

public:
    VoxelGrid(MetaMesh &mesh, float spacing);

    /**
     * Outline the voxels that contain quads.
     */
    void drawDebug();

    /**
     * Hit test a ray against the mesh. Will only consider voxels that the
     * ray passes through.
     */
    bool hitTest(const Vector3 &origin, const Vector3 &ray, HitTest &result) const;

    /**
     * Fills vertices with all vertices of the voxels in the axis-aligned
     * bounding box, and also in the extra voxel if the AABB is partially
     * or entirely outside of the voxel grid.
     */
    void getVerticesInAABB(const Vector3 &minCoord, const Vector3 &maxCoord, QSet<MetaVertex *> &vertices);

    /**
     * Redistributes all vertices in changedVertices and all quads touching
     * a vertex in changedVertices.
     */
    virtual void updateVertices(const QSet<MetaVertex *> &changedVertices);
};

#endif // MESHACCELERATION_H
