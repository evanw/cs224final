#include "catmullclark.h"


CatmullFace::CatmullFace(int numSides) : n(numSides) {
    for (int i = 0; i < numSides; ++i) {
        neighbors += NULL;
        points += -1;
        edgePoints += -1;
    }
}


CatmullMesh::CatmullMesh(const Mesh &m) {
    foreach (const Vertex &v, m.vertices) {
        CatmullVertex cv;
        cv.pos = v.pos;
        cv.normal = v.normal;
        vertices += cv;
    }

    // create the faces with vertices only
    foreach (const Triangle &tri, m.triangles) {
        CatmullFace cTri(3);
        // set vertices (copies made since Catmull vertex positions will be changed)
        cTri.points[0] = tri.a.index;
        cTri.points[1] = tri.b.index;
        cTri.points[2] = tri.c.index;
        faces += cTri;
    }
    foreach (const Quad &quad, m.quads) {
        CatmullFace cQuad(4);
        cQuad.points[0] = quad.a.index;
        cQuad.points[1] = quad.b.index;
        cQuad.points[2] = quad.c.index;
        cQuad.points[3] = quad.d.index;
        faces += cQuad;
    }

    // add the neighbor and face points
    for (int i = 0; i < faces.size(); ++i) {
        CatmullFace &face = faces[i];
        fillNeighbors(face);

        Vertex fp;
        // set face point to average of vertex points
        for (int i = 0; i < face.n; ++i) {
            fp.pos += vertices[face.points[1]].pos;
        }
        fp.pos /= face.n;
        face.facePoint = facePoints.size();
        facePoints += fp;
    }

    // add the edge points: edgePoints[0] on 0-1 edge, edgePoints[1] on 1-2 edge, etc.
    for (int i = 0; i < faces.size(); ++i) {
        CatmullFace &f = faces[i];
        // edgePt position is the average of the edge's vertices and the face points of the two adjacent faces
        for (int i = 0; i < f.n; ++i) {
            Vertex v;
            v.pos = (vertices[f.points[i]].pos + vertices[f.points[(i + 1) % f.n]].pos +
                     facePoints[f.facePoint].pos + facePoints[f.neighbors[i]->facePoint].pos) / f.n;
            f.edgePoints[i] = edgePoints.size();
            edgePoints += v;
        }
    }
}

// find the neighboring face for each edge of 'face'
void CatmullMesh::fillNeighbors(CatmullFace &face) {
    for (int i = 0; i < face.n; ++i) {
        bool neighborFound = false;
        // for each potential neighbor
        foreach (const CatmullFace &neighbor, faces) {
            for (int j = 0; j < neighbor.n; ++j) {
                // if ordering of vertices is the same
                if ((face.points[i] < face.points[(i + 1) % face.n]) == (neighbor.points[j] < neighbor.points[(j + 1) % neighbor.n])) {
                    if (face.points[i] == neighbor.points[j] && face.points[(i + 1) % face.n] == neighbor.points[(j + 1) % neighbor.n]) {
                        face.neighbors[i] = &neighbor;
                        neighborFound = true;
                        break;
                    }
                } else if (face.points[i] == neighbor.points[(j + 1) % neighbor.n] && face.points[(i + 1) % face.n] == neighbor.points[j]) {
                    // same edge, vertices in opposite order
                    face.neighbors[i] = &neighbor;
                    neighborFound = true;
                    break;
                }
            }

            if (neighborFound) break;
        }
    }
}

// update the mesh vertices (step 3 of Catmull-Clark subdivision)
bool CatmullMesh::moveVertices() {
    // find neighboring edges and faces
    foreach (const CatmullFace &face, faces) {
        for (int i = 0; i < face.n; ++i) {
            int prevIndex = (i == 0 ? face.n - 1 : i - 1);
            CatmullVertex &v = vertices[face.points[i]];
            v.facePoints += facePoints[face.facePoint].pos;
            if (!v.edgePoints.contains(edgePoints[face.edgePoints[i]].pos)) {
                v.edgePoints += edgePoints[face.edgePoints[i]].pos;
            }
            if (!v.edgePoints.contains(edgePoints[face.edgePoints[prevIndex]].pos)) {
                v.edgePoints += edgePoints[face.edgePoints[prevIndex]].pos;
            }
        }
    }

    // take averages for faces and edges and move vertices
    Vector3 faceAverage, edgeAverage;
    for (int i = 0; i < vertices.size(); ++i) {
        CatmullVertex &v = vertices[i];
        if (v.edgePoints.size() != v.facePoints.size()) return false;
        int numNeighbors = v.edgePoints.size();
        faceAverage.x = faceAverage.y = faceAverage.z = 0;
        edgeAverage.x = edgeAverage.y = edgeAverage.z = 0;

        for (int i = 0; i < numNeighbors; ++i) {
            faceAverage += v.facePoints[i];
            edgeAverage += v.edgePoints[i];
        }

        // Pnew = ( F + 2R + (n - 3)P ) / n
        v.pos = (faceAverage + edgeAverage * 2 + v.pos * (numNeighbors - 3)) / numNeighbors;
    }

    return true;
}


bool CatmullMesh::convertToMesh(Mesh &m) {
    // create the list of vertices
    foreach (const CatmullVertex &cv, vertices) {
        Vertex v;
        v.pos = cv.pos;
        v.normal = cv.normal;
        m.vertices += v;
    }

    int edgeOffset = m.vertices.size();
    foreach (const Vertex &cv, edgePoints) {
        m.vertices += cv;
    }

    int faceOffset = m.vertices.size();
    foreach (const Vertex &cv, facePoints) {
        m.vertices += cv;
    }

    // create the lists of quads and triangles
    foreach (const CatmullFace &face, faces) {
        for (int i = 0; i < face.n; ++i) {
            Quad q;
            q.a = face.points[i];
            q.b = edgeOffset + face.edgePoints[i];
            q.c = faceOffset + face.facePoint;
            q.d = edgeOffset + face.edgePoints[(i + face.n - 1) % face.n];
            m.quads += q;
        }
    }

    return true;
}


bool CatmullMesh::subdivide(const Mesh &in, Mesh &out) {
    CatmullMesh cm(in);
    if (!cm.moveVertices()) return false;

    out.balls = in.balls;
    return cm.convertToMesh(out);
}
