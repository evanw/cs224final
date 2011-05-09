#include "catmullclark.h"


void averageJointWeights(const CatmullVertex &v1, const CatmullVertex &v2,
                         int *dstIndices, float *dstWeights) {
    if (v1.jointIndices[0] == v2.jointIndices[0]) {
        std::copy(v1.jointIndices, v1.jointIndices + 2, dstIndices);
        std::copy(v1.jointWeights, v1.jointWeights + 2, dstWeights);
    } else {
        dstIndices[0] = v1.jointIndices[0];
        dstIndices[1] = v2.jointIndices[0];
        dstWeights[0] = dstWeights[1] = 0.5f;
    }
}


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
        std::copy(v.jointIndices, v.jointIndices + 2, cv.jointIndices);
        std::copy(v.jointWeights, v.jointWeights + 2, cv.jointWeights);
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
    for (int iface = 0; iface < faces.size(); ++iface) {
        CatmullFace &face = faces[iface];

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

        // set the bone weights for animation
        const CatmullVertex &v = vertices[face.points[0]];
        // TODO: Fix weights, can it be split between 3 bones?
        if (face.n == 4) {
            const CatmullVertex &v2 = vertices[face.points[2]];
            averageJointWeights(v, v2, fp.jointIndices, fp.jointWeights);
        } else {
            std::copy(v.jointIndices, v.jointIndices + 2, fp.jointIndices);
            std::copy(v.jointWeights, v.jointWeights + 2, fp.jointWeights);
        }


        face.facePoint = facePoints.size();
        facePoints += fp;
    }

    // add the edge points
    QMap<QPair<int, int>, CatmullEdge>::const_iterator it;
    for (it = edges.begin(); it != edges.end(); ++it) {
        CatmullEdge &edge = edges[it.key()];

        if (it.value().faces[1] == NULL) {
            // for edges on the border of a hole, the edge point is average of edge endpoints
            edge.pos = (vertices[it.key().first].pos + vertices[it.key().second].pos) / 2;

            // set the weights for animation
            averageJointWeights(vertices[it.key().first], vertices[it.key().second], edge.jointIndices, edge.jointWeights);
        } else {
            // edge point is average of edge endpoints and adjacent face points
            edges[it.key()].pos = (vertices[it.key().first].pos + vertices[it.key().second].pos +
                               facePoints[it.value().faces[0]->facePoint].pos + facePoints[it.value().faces[1]->facePoint].pos) / 4;

            // TODO: Fix edge animation weights
            averageJointWeights(vertices[it.key().first], vertices[it.key().second], edge.jointIndices, edge.jointWeights);
        }
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
            if (!v.edges.contains(&edges[face.edges[i]])) {
                v.edges += &edges[face.edges[i]];
            }
            if (!v.edges.contains(&edges[face.edges[prevIndex]])) {
                v.edges += &edges[face.edges[prevIndex]];
            }
        }
    }

    // take averages for faces and edges and move vertices
    Vector3 faceAverage, edgeAverage;
    for (int i = 0; i < vertices.size(); ++i) {
        CatmullVertex &v = vertices[i];
        faceAverage.x = faceAverage.y = faceAverage.z = 0;
        edgeAverage.x = edgeAverage.y = edgeAverage.z = 0;

        if (v.edges.size() != v.facePoints.size()) {
            // point is on the border of a hole, average edges along hole and old point
            int count = 0;
            foreach (const CatmullEdge *edge, v.edges) {
                if (edge->faces[1] == NULL) {
                    edgeAverage += edge->pos;
                    ++count;
                }
            }
            v.pos = (edgeAverage + v.pos) / (count + 1);
        } else {
            int numNeighbors = v.edges.size();

            for (int i = 0; i < numNeighbors; ++i) {
                faceAverage += v.facePoints[i];
                edgeAverage += v.edges[i]->pos;
            }
            faceAverage /= numNeighbors;
            edgeAverage /= numNeighbors;

            // Pnew = ( F + 2R + (n - 3)P ) / n
            v.pos = (faceAverage + edgeAverage * 2 + v.pos * (numNeighbors - 3)) / numNeighbors;
        }
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
        std::copy(cv.jointIndices, cv.jointIndices + 2, v.jointIndices);
        std::copy(cv.jointWeights, cv.jointWeights + 2, v.jointWeights);
        m.vertices += v;
    }

    QMap<QPair<int, int>, int> edgeIndices;
    QMap<QPair<int, int>, CatmullEdge>::const_iterator it;
    for (it = edges.begin(); it != edges.end(); ++it) {
        const CatmullEdge &edge = it.value();
        edgeIndices[it.key()] = m.vertices.size();
        Vertex v;
        v.pos = edge.pos;
        std::copy(edge.jointIndices, edge.jointIndices + 2, v.jointIndices);
        std::copy(edge.jointWeights, edge.jointWeights + 2, v.jointWeights);
        m.vertices += v;
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
