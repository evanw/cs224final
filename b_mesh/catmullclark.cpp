#include "catmullclark.h"



QPair<int, int> getEdgeIndex(int v1, int v2) {
    return v1 < v2 ? QPair<int, int>(v1, v2) : QPair<int, int>(v2, v1);
}


CatmullFace::CatmullFace(int numSides) : n(numSides) {
    for (int i = 0; i < numSides; ++i) {
        neighbors += NULL;
        points += -1;
        edges += QPair<int, int>();
    }
}


CatmullMesh::CatmullMesh(const Mesh &m) {
    // copy the vertices over
    foreach (const Vertex &v, m.vertices) {
        CatmullVertex cv;
        cv.pos = v.pos;
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

    // add the edges and face points
    for (int i = 0; i < faces.size(); ++i) {
        CatmullFace &face = faces[i];

        // add edges
        for (int i = 0; i < face.n; ++i) {
            const QPair<int, int>& pair = getEdgeIndex(face.points[i], face.points[(i + 1) % face.n]);
            face.edges[i] = pair;
            if (!edges.contains(pair)) {
                edges[pair].faces[0] = &face;
            } else if (edges[pair].faces[1] == NULL) {
                edges[pair].faces[1] = &face;
            } else {
                std::cerr << "Error, edge connects to more than 2 faces" << std::endl;
                valid = false;
                return;
            }
        }

        // add face point
        Vertex fp;
        fp.pos = Vector3();
        // set face point to average of vertex points
        for (int i = 0; i < face.n; ++i) {
            fp.pos += vertices[face.points[i]].pos;
        }
        fp.pos /= face.n;
        face.facePoint = facePoints.size();
        facePoints += fp;
    }

    // add the edge points
    QMap<QPair<int, int>, CatmullEdge>::const_iterator it;
    for (it = edges.begin(); it != edges.end(); ++it) {
        if (it.value().faces[1] == NULL) {
            std::cerr << "Error, edge connects to only one face" << std::endl;
            valid = false;
            return;
        }
        edges[it.key()].pos = (vertices[it.key().first].pos + vertices[it.key().second].pos +
                               facePoints[it.value().faces[0]->facePoint].pos + facePoints[it.value().faces[1]->facePoint].pos) / 4;
    }

    valid = true;
}

// update the mesh vertices (step 3 of Catmull-Clark subdivision)
bool CatmullMesh::moveVertices() {
    // find neighboring edges and faces
    foreach (const CatmullFace &face, faces) {
        for (int i = 0; i < face.n; ++i) {
            int prevIndex = (i == 0 ? face.n - 1 : i - 1);
            CatmullVertex &v = vertices[face.points[i]];
            v.facePoints += facePoints[face.facePoint].pos;
            if (!v.edgePoints.contains(edges[face.edges[i]].pos)) {
                v.edgePoints += edges[face.edges[i]].pos;
            }
            if (!v.edgePoints.contains(edges[face.edges[prevIndex]].pos)) {
                v.edgePoints += edges[face.edges[prevIndex]].pos;
            }
        }
    }

    // take averages for faces and edges and move vertices
    Vector3 faceAverage, edgeAverage;
    for (int i = 0; i < vertices.size(); ++i) {
        CatmullVertex &v = vertices[i];
        if (v.edgePoints.size() != v.facePoints.size()) {
            std::cerr << "Valence mismatch error: " << v.edgePoints.size() << " edges, " << v.facePoints.size() << " faces" << std::endl;
            return false;
        }
        int numNeighbors = v.edgePoints.size();
        faceAverage.x = faceAverage.y = faceAverage.z = 0;
        edgeAverage.x = edgeAverage.y = edgeAverage.z = 0;

        for (int i = 0; i < numNeighbors; ++i) {
            faceAverage += v.facePoints[i];
            edgeAverage += v.edgePoints[i];
        }
        faceAverage /= numNeighbors;
        edgeAverage /= numNeighbors;

        // Pnew = ( F + 2R + (n - 3)P ) / n
        v.pos = (faceAverage + edgeAverage * 2 + v.pos * (numNeighbors - 3)) / numNeighbors;
    }

    return true;
}


bool CatmullMesh::convertToMesh(Mesh &m) {
    // clear the old mesh
    m.vertices.clear();
    m.triangles.clear();
    m.quads.clear();

    // create the list of vertices
    foreach (const CatmullVertex &cv, vertices) {
        Vertex v;
        v.pos = cv.pos;
        m.vertices += v;
    }

    QMap<QPair<int, int>, int> edgeIndices;
    QMap<QPair<int, int>, CatmullEdge>::const_iterator it;
    for (it = edges.begin(); it != edges.end(); ++it) {
        edgeIndices[it.key()] = m.vertices.size();
        m.vertices += it.value().pos;
    }

    int faceOffset = m.vertices.size();
    foreach (const Vertex &cv, facePoints) {
        m.vertices += cv;
    }

    // create the lists of quads and triangles
    foreach (const CatmullFace &face, faces) {
        int faceIndex = faceOffset + face.facePoint;
        for (int i = 0; i < face.n; ++i) {
            Quad q;
            q.a = face.points[i];
            q.b = edgeIndices[face.edges[i]];
            q.c = faceIndex;
            q.d = edgeIndices[face.edges[(i + face.n - 1) % face.n]];
            m.quads += q;
        }
    }

    return true;
}


bool CatmullMesh::subdivide(const Mesh &in, Mesh &out) {
    CatmullMesh cm(in);
    if (!cm.valid) return false;
    if (!cm.moveVertices()) return false;
    out.balls = in.balls;
    if (!cm.convertToMesh(out)) return false;
    out.updateNormals();
    return true;
}
