#include "meshacceleration.h"
#include "geometry.h"
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
    int x = floorf(gridPos.x);
    int y = floorf(gridPos.y);
    int z = floorf(gridPos.z);
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

void VoxelGrid::hitTestHelper(Quad *quad, const Vector3 &origin, const Vector3 &ray, HitTest &result) const
{
    const Vector3 &a = mesh.vertices[quad->a.index]->pos;
    const Vector3 &b = mesh.vertices[quad->b.index]->pos;
    const Vector3 &c = mesh.vertices[quad->c.index]->pos;
    const Vector3 &d = mesh.vertices[quad->d.index]->pos;

#ifdef VOXEL_DEBUG
    float prevT = result.t;
#endif

    HitTest tempResult;
    if (Raytracer::hitTestTriangle(a, b, c, origin, ray, tempResult)) result.mergeWith(tempResult);
    if (Raytracer::hitTestTriangle(a, c, d, origin, ray, tempResult)) result.mergeWith(tempResult);

#ifdef VOXEL_DEBUG
    if (result.t != prevT)
    {
        glBegin(GL_LINE_LOOP);
        glVertex3fv(a.xyz);
        glVertex3fv(b.xyz);
        glVertex3fv(c.xyz);
        glVertex3fv(d.xyz);
        glEnd();
    }
#endif
}

void VoxelGrid::addQuadToVoxels(Quad &quad)
{
    // Load the quad
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

    // Add the quad to all voxels its aabb overlaps (this handles
    // out-of-bounds errors via a special case in getIndex)
    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {
            for (int z = minZ; z <= maxZ; z++)
            {
                int index = getIndex(x, y, z);
                voxels[index].quads += &quad;
                voxelsForQuad[&quad] += index;
            }
        }
    }
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

    // Expand the AABB a little to account for some user interaction
    Vector3 center((minCorner + maxCorner) / 2);
    minCorner += (minCorner - center) * 0.25f;
    maxCorner += (maxCorner - center) * 0.25f;

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
    {
        int index = getVoxelForPos(vertex->pos);
        voxels[index].vertices += vertex;
        vertex->accelData = index;
    }

    // Place quads in voxels
    for (int i = 0; i < mesh.mesh.quads.count(); i++)
        addQuadToVoxels(mesh.mesh.quads[i]);
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

bool VoxelGrid::hitTest(const Vector3 &origin, const Vector3 &ray, HitTest &result) const
{
    QSet<Quad *> alreadyTested;

    // Start off intersecting nothing
    result.t = FLT_MAX;
    if (voxels.isEmpty())
        return false;

    // Process extra voxel first
    const Voxel &extra = voxels[countX * countY * countZ];
    foreach (Quad *quad, extra.quads)
    {
        if (alreadyTested.contains(quad)) continue;
        hitTestHelper(quad, origin, ray, result);
        alreadyTested += quad;
    }

    // This uses the slab intersection method
    Vector3 tMin = (minCorner - origin) / ray;
    Vector3 tMax = (maxCorner - origin) / ray;
    Vector3 t1 = Vector3::min(tMin, tMax);
    Vector3 t2 = Vector3::max(tMin, tMax);
    float tNear = t1.max();
    float tFar = t2.min();

    // If the line segment doesn't hit the cube or the cube is behind us
    if (tNear > tFar || tFar < 0)
        return result.t < FLT_MAX;

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
    float tMaxX = ((stepX > 0 ? x + 1 : x) - start.x) / gridSpaceRay.x;
    float tMaxY = ((stepY > 0 ? y + 1 : y) - start.y) / gridSpaceRay.y;
    float tMaxZ = ((stepZ > 0 ? z + 1 : z) - start.z) / gridSpaceRay.z;
    float tDeltaX = fabsf(1 / gridSpaceRay.x);
    float tDeltaY = fabsf(1 / gridSpaceRay.y);
    float tDeltaZ = fabsf(1 / gridSpaceRay.z);

    // Trace the ray through the grid
    while (1)
    {
#ifdef VOXEL_DEBUG
        Vector3 minPos = convertFromGrid(Vector3(x, y, z));
        Vector3 maxPos = convertFromGrid(Vector3(x + 1, y + 1, z + 1));
        drawWireCube(minPos, maxPos);
#endif

        // Process the current voxel
        const Voxel &voxel = voxels[getInternalIndex(x, y, z)];
        foreach (Quad *quad, voxel.quads)
        {
            if (alreadyTested.contains(quad)) continue;
            hitTestHelper(quad, origin, ray, result);
            alreadyTested += quad;
        }

        // Stop the trace if we would move past the closest intersection
        float tMin = min(tMaxX, min(tMaxY, tMaxZ));
        if (result.t < tMin)
            break;

        // Advance to the next voxel and stop if we leave the grid
        if (tMin == tMaxX)
        {
            x += stepX;
            if (x == outX) break;
            tMaxX += tDeltaX;
        }
        else if (tMin == tMaxY)
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

    return result.t < FLT_MAX;
}

void VoxelGrid::getVerticesInAABB(const Vector3 &minCoord, const Vector3 &maxCoord, QSet<MetaVertex *> &vertices)
{
    // Calculate the integer volume
    Vector3 minGrid = convertToGrid(minCoord);
    Vector3 maxGrid = convertToGrid(maxCoord);
    int minX = floorf(minGrid.x);
    int minY = floorf(minGrid.y);
    int minZ = floorf(minGrid.z);
    int maxX = floorf(maxGrid.x);
    int maxY = floorf(maxGrid.y);
    int maxZ = floorf(maxGrid.z);

    // Clamp the volume to the bounds of the grid
    bool isOutside = false;
    if (minX < 0) { minX = 0; isOutside = true; }
    if (minY < 0) { minY = 0; isOutside = true; }
    if (minZ < 0) { minZ = 0; isOutside = true; }
    if (maxX >= countX) { maxX = countX - 1; isOutside = true; }
    if (maxY >= countY) { maxY = countY - 1; isOutside = true; }
    if (maxZ >= countZ) { maxZ = countZ - 1; isOutside = true; }

    // Add all vertices in the volume
    vertices.clear();
    for (int x = minX; x <= maxX; x++)
        for (int y = minY; y <= maxY; y++)
            for (int z = minZ; z <= maxZ; z++)
                vertices += voxels[getInternalIndex(x, y, z)].vertices;
    if (isOutside)
        vertices += voxels[countX * countY * countZ].vertices;
}

void VoxelGrid::updateVertices(const QSet<MetaVertex *> &changedVertices)
{
    QSet<Quad *> affectedQuads;

    // Move the vertices to new voxels
    foreach (MetaVertex *vertex, changedVertices)
    {
        int oldIndex = vertex->accelData;
        int newIndex = getVoxelForPos(vertex->pos);

        // Move the vertex to a new voxel
        if (oldIndex != newIndex)
        {
            voxels[oldIndex].vertices -= vertex;
            voxels[newIndex].vertices += vertex;
            vertex->accelData = newIndex;
        }

        // Also mark all affected quads
        foreach (Quad *quad, vertex->neighbors)
            affectedQuads += quad;
    }

    // Move all affected quads to new voxels
    foreach (Quad *quad, affectedQuads)
    {
        // Remove quad from the old voxels
        foreach (int index, voxelsForQuad[quad])
            voxels[index].quads -= quad;
        voxelsForQuad[quad].clear();

        // Add quad to the new voxels
        addQuadToVoxels(*quad);
    }
}
