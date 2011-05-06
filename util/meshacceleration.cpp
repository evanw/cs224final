#include "meshacceleration.h"
#include "geometry.h"
#include <float.h>
#include <qgl.h>

Vector3 VoxelGrid::convertToGrid(const Vector3 &pos) const
{
    return (pos - minCorner) * Vector3(countX, countY, countZ) / (maxCorner - minCorner);
}

Vector3 VoxelGrid::convertFromGrid(const Vector3 &pos) const
{
    return minCorner + (maxCorner - minCorner) * pos / Vector3(countX, countY, countZ);
}

int VoxelGrid::getVoxelForPos(const Vector3 &pos) const
{
    Vector3 gridPos = convertToGrid(pos);
    int x = floorf(gridPos.x * countX);
    int y = floorf(gridPos.y * countY);
    int z = floorf(gridPos.z * countZ);
    return getIndex(x, y, z);
}

int VoxelGrid::getIndex(int x, int y, int z) const
{
    if (x >= 0 && x < countX && y >= 0 && y < countY && z >= 0 && z < countZ)
        return getInternalIndex(x, y, z);
    return countX * countY * countZ;
}

int VoxelGrid::getInternalIndex(int x, int y, int z) const
{
    return x + (y + z * countY) * countX;
}

VoxelGrid::VoxelGrid(MetaMesh &mesh, float spacing) : AccelerationDataStructure(mesh), countX(0), countY(0), countZ(0)
{
    if (mesh.vertices.isEmpty())
        return;

    // Compute the axis-aligned bounding-box over all the vertices
    minCorner = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    maxCorner = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    foreach (MetaVertex *vertex, mesh.vertices)
    {
        minCorner = Vector3::min(minCorner, vertex->pos);
        maxCorner = Vector3::max(maxCorner, vertex->pos);
    }

    // Generate the grid. We do this once at the start instead of
    // every time because we assume that the mesh won't move much.
    // Any things that move outside the bounds will go in the
    // extra voxel at the end.
    Vector3 delta((maxCorner - minCorner) / spacing);
    countX = ceilf(delta.x);
    countY = ceilf(delta.y);
    countZ = ceilf(delta.z);
    voxels.resize(countX * countY * countZ + 1);

    // Place vertices in voxels
    foreach (MetaVertex *vertex, mesh.vertices)
        voxels[getVoxelForPos(vertex->pos)].vertices += vertex;

    // Place quads in voxels
    for (int i = 0; i < mesh.mesh.quads.count(); i++)
    {
        // Load the quad
        Quad &quad = mesh.mesh.quads[i];
        const Vector3 &a = mesh.vertices[quad.a.index]->pos;
        const Vector3 &b = mesh.vertices[quad.b.index]->pos;
        const Vector3 &c = mesh.vertices[quad.c.index]->pos;
        const Vector3 &d = mesh.vertices[quad.d.index]->pos;

        // Find the axis-aligned bounding-box of the quad
        Vector3 minPos = a;
        minPos = Vector3::min(minPos, b);
        minPos = Vector3::min(minPos, c);
        minPos = Vector3::min(minPos, d);
        minPos = convertToGrid(minPos);
        Vector3 maxPos = a;
        maxPos = Vector3::max(maxPos, b);
        maxPos = Vector3::max(maxPos, c);
        maxPos = Vector3::max(maxPos, d);
        maxPos = convertToGrid(maxPos);
        int minX = floorf(minPos.x);
        int minY = floorf(minPos.y);
        int minZ = floorf(minPos.z);
        int maxX = floorf(maxPos.x);
        int maxY = floorf(maxPos.y);
        int maxZ = floorf(maxPos.z);

        // Add the quad to all voxels its aabb overlaps
        for (int x = minX; x <= maxX; x++)
            for (int y = minY; y <= maxY; y++)
                for (int z = minZ; z <= maxZ; z++)
                    voxels[getIndex(x, y, z)].quads += &quad;
    }
}

void VoxelGrid::drawDebug()
{
    for (int x = 0; x < countX; x++)
    {
        for (int y = 0; y < countY; y++)
        {
            for (int z = 0; z < countZ; z++)
            {
                Voxel &voxel = voxels[getInternalIndex(x, y, z)];
                if (voxel.quads.isEmpty())
                    continue;

                drawWireCube(convertFromGrid(Vector3(x, y, z)), convertFromGrid(Vector3(x + 1, y + 1, z + 1)));
            }
        }
    }
}

void VoxelGrid::updateVertices(const QVector<MetaVertex *> &vertices)
{
    // http://www.dia.unisa.it/~cosenza/papers/EGITA08grid.pdf
    // TODO: fixed grid density???
}

bool VoxelGrid::hitTest(const Vector3 &origin, const Vector3 &ray, HitTest &result) const
{
    // Process extra voxel first
    const Voxel &extra = voxels[countX * countY * countZ];
    // TODO: process extra

    // This uses the slab intersection method
    Vector3 tMin = (minCorner - origin) / ray;
    Vector3 tMax = (maxCorner - origin) / ray;
    Vector3 t1 = Vector3::min(tMin, tMax);
    Vector3 t2 = Vector3::max(tMin, tMax);
    float tNear = t1.max();
    float tFar = t2.min();

    // If the line segment doesn't hit the cube or the cube is behind us
    if (tNear > tFar || tFar < 0)
        return false;

    // Algorithm from paper: A Fast Voxel Traversal Algorithm for Ray Tracing
    Vector3 start = convertToGrid(origin + ray * max(0, tNear));
    int x = std::max(0, std::min((int)floorf(start.x), countX - 1));
    int y = std::max(0, std::min((int)floorf(start.y), countY - 1));
    int z = std::max(0, std::min((int)floorf(start.z), countZ - 1));
    int stepX = (ray.x > 0) ? 1 : -1;
    int stepY = (ray.y > 0) ? 1 : -1;
    int stepZ = (ray.z > 0) ? 1 : -1;
    int outX = (stepX > 0) ? countX : -1;
    int outY = (stepY > 0) ? countY : -1;
    int outZ = (stepZ > 0) ? countZ : -1;
    Vector3 gridSpaceRay = convertToGrid(minCorner + ray);
    float tMaxX = ((stepX > 0 ? ceilf : floorf)(start.x) - start.x) / gridSpaceRay.x;
    float tMaxY = ((stepY > 0 ? ceilf : floorf)(start.y) - start.y) / gridSpaceRay.y;
    float tMaxZ = ((stepZ > 0 ? ceilf : floorf)(start.z) - start.z) / gridSpaceRay.z;
    float tDeltaX = fabsf(1 / gridSpaceRay.x);
    float tDeltaY = fabsf(1 / gridSpaceRay.y);
    float tDeltaZ = fabsf(1 / gridSpaceRay.z);

    // Trace the ray through the grid
    while (1)
    {
        // Process the current voxel
        const Voxel &voxel = voxels[getInternalIndex(x, y, z)];
        // TODO: process voxel

        // Advance to the next voxel
        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                x += stepX;
                if (x == outX) break;
                tMaxX += tDeltaX;
            }
            else
            {
                z += stepZ;
                if (z == outZ) break;
                tMaxZ += tDeltaZ;
            }
        }
        else if (tMaxY < tMaxZ)
        {
            y += stepY;
            if (y == outY) break;
            tMaxY += tDeltaY;
        }
        else
        {
            z += stepZ;
            if (z == outZ) break;
            tMaxZ += tDeltaZ;
        }
    }

    return false;
}

void VoxelGrid::getVerticesInSphere(const Vector3 &center, float radius, QVector<MetaVertex *> &vertices) const
{
    vertices.clear();
}
