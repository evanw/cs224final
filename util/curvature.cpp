#include "curvature.h"
#include "matrix.h"
#include "mesh.h"

// fill in u and v given w
void generateComplementBasis(Vector3& u, Vector3& v, const Vector3& w) {
    float invLength;

    if (fabsf(w.x) >= fabsf(w.y)) {
        // w.x or w.z is the largest magnitude component, swap them
        invLength = 1.f / sqrtf(w.x * w.x + w.z * w.z);
        u.x = -w.z * invLength;
        u.y = 0.f;
        u.z = w.x * invLength;
        v.x = w.y * u.z;
        v.y = w.z * u.x - w.x * u.z;
        v.z = -w.y * u.x;
    } else {
        // w.y or w.z is the largest magnitude component, swap them
        invLength = 1.f / sqrtf(w.y * w.y + w.z * w.z);
        u.x = 0.f;
        u.y = w.z * invLength;
        u.z = -w.y * invLength;
        v.x = w.y * u.z - w.z * u.y;
        v.y = -w.x * u.z;
        v.z = w.x * u.y;
    }
}


Curvature::Curvature()
{
}


void Curvature::computeCurvatures(const Mesh &mesh) {
    int nVerts = mesh.vertices.size();

    // Compute the matrix of normal derivatives.
    QVector<Matrix3> DNormal(nVerts);
    QVector<Matrix3> WWTrn(nVerts);
    QVector<Matrix3> DWTrn(nVerts);
    bool DWTrnZero[nVerts];

    int row, col;
    foreach (const Triangle &t, mesh.triangles) {
        // Get vertex indices.
        int V[3];
        V[0] = t.a.index;
        V[1] = t.b.index;
        V[2] = t.c.index;

        for (int j = 0; j < 3; ++j) {
            int i0 = V[j];
            const Vertex &v0 = mesh.vertices[V[j]];
            const Vertex &v1 = mesh.vertices[V[(j+1) % 3]];
            const Vertex &v2 = mesh.vertices[V[(j+2) % 3]];

            // Compute edge from V0 to V1, project to tangent plane of vertex,
            // and compute difference of adjacent normals.
            Vector3 E = v1.pos - v0.pos;
            Vector3 W = E - (E.dot(v0.normal)) * v0.normal;
            Vector3 D = v1.normal - v0.normal;
            for (row = 0; row < 3; ++row) {
                for (col = 0; col < 3; ++col) {
                    WWTrn[i0][row][col] += W.xyz[row] * W.xyz[col];
                    DWTrn[i0][row][col] += D.xyz[row] * W.xyz[col];
                }
            }

            // Compute edge from V0 to V2, project to tangent plane of vertex,
            // and compute difference of adjacent normals.
            E = v2.pos - v0.pos;
            W = E - (E.dot(v0.normal)) * v0.normal;
            D = v2.normal - v0.normal;
            for (row = 0; row < 3; ++row) {
                for (col = 0; col < 3; ++col) {
                    WWTrn[i0][row][col] += W.xyz[row] * W.xyz[col];
                    DWTrn[i0][row][col] += D.xyz[row] * W.xyz[col];
                }
            }
        }
    }

    // Add in N*N^T to W*W^T for numerical stability.  In theory 0*0^T gets
    // added to D*W^T, but of course no update is needed in the
    // implementation.  Compute the matrix of normal derivatives.
    for (int i = 0; i < nVerts; ++i) {
        for (row = 0; row < 3; ++row) {
            for (col = 0; col < 3; ++col) {
                WWTrn[i][row][col] = 0.5f * WWTrn[i][row][col] +
                    mesh.vertices[i].normal.xyz[row] * mesh.vertices[i].normal.xyz[col];
                DWTrn[i][row][col] *= 0.5f;
            }
        }

        // Compute the max-abs entry of D*W^T.  If this entry is (nearly)
        // zero, flag the DNormal matrix as singular.
        float maxAbs = (float)0;
        for (row = 0; row < 3; ++row) {
            for (col = 0; col < 3; ++col) {
                float absEntry = fabsf(DWTrn[i][row][col]);
                if (absEntry > maxAbs) {
                    maxAbs = absEntry;
                }
            }
        }
        if (maxAbs < 1e-07) {
            DWTrnZero[i] = true;
        }

        DNormal[i] = DWTrn[i]*WWTrn[i].inverse();
    }

    // If N is a unit-length normal at a vertex, let U and V be unit-length
    // tangents so that {U, V, N} is an orthonormal set.  Define the matrix
    // J = [U | V], a 3-by-2 matrix whose columns are U and V.  Define J^T
    // to be the transpose of J, a 2-by-3 matrix.  Let dN/dX denote the
    // matrix of first-order derivatives of the normal vector field.  The
    // shape matrix is
    //   S = (J^T * J)^{-1} * J^T * dN/dX * J = J^T * dN/dX * J
    // where the superscript of -1 denotes the inverse.  (The formula allows
    // for J built from non-perpendicular vectors.) The matrix S is 2-by-2.
    // The principal curvatures are the eigenvalues of S.  If k is a principal
    // curvature and W is the 2-by-1 eigenvector corresponding to it, then
    // S*W = k*W (by definition).  The corresponding 3-by-1 tangent vector at
    // the vertex is called the principal direction for k, and is J*W.
    minCurvatures.resize(nVerts);
    maxCurvatures.resize(nVerts);
    minDirections.resize(nVerts);
    maxDirections.resize(nVerts);

    for (int i = 0; i < nVerts; ++i) {
        // Compute U and V given N.
        Vector3 U, V;
        generateComplementBasis(U, V, mesh.vertices[i].normal);

        if (DWTrnZero[i]) {
            // At a locally planar point.
            minCurvatures[i] = (float)0;
            maxCurvatures[i] = (float)0;
            minDirections[i] = U;
            maxDirections[i] = V;
            continue;
        }

        // Compute S = J^T * dN/dX * J.  In theory S is symmetric, but
        // because we have estimated dN/dX, we must slightly adjust our
        // calculations to make sure S is symmetric.
        float s01 = U.dot(DNormal[i] * V);
        float s10 = V.dot(DNormal[i] * U);
        float sAvr = 0.5f * (s01 + s10);
        float S[2][2];
        S[0][0] = U.dot(DNormal[i]*U);
        S[0][1] = sAvr;
        S[1][0] = sAvr;
        S[1][1] = V.dot(DNormal[i]*V);

        // Compute the eigenvalues of S (min and max curvatures).
        float trace = S[0][0] + S[1][1];
        float det = S[0][0] * S[1][1] - S[0][1] * S[1][0];
        float discr = trace * trace - 4.0f * det;
        float rootDiscr = sqrtf(fabsf(discr));
        minCurvatures[i] = 0.5f * (trace - rootDiscr);
        maxCurvatures[i] = 0.5f * (trace + rootDiscr);

        // Compute the eigenvectors of S.
        Vector2 W0(S[0][1], minCurvatures[i] - S[0][0]);
        Vector2 W1(minCurvatures[i] - S[1][1], S[1][0]);
        if (W0.lengthSquared() >= W1.lengthSquared()) {
            W0.normalize();
            minDirections[i] = W0.x * U + W0.y * V;
        } else {
            W1.normalize();
            minDirections[i] = W1.x * U + W1.y * V;
        }

        W0 = Vector2(S[0][1], maxCurvatures[i] - S[0][0]);
        W1 = Vector2(maxCurvatures[i] - S[1][1], S[1][0]);
        if (W0.lengthSquared() >= W1.lengthSquared()) {
            W0.normalize();
            maxDirections[i] = W0.x * U + W0.y * V;
        } else {
            W1.normalize();
            maxDirections[i] = W1.x * U + W1.y * V;
        }
    }
}

/* const float* Curvature<float>::GetMinCurvatures() const
{
    return mMinCurvatures;
}

const float* Curvature<float>::GetMaxCurvatures() const
{
    return mMaxCurvatures;
}

const Vector3<float>* Curvature::getMinDirections() const
{
    return mMinDirections;
}

const Vector3<float>* Curvature::getMaxDirections() const
{
    return mMaxDirections;
} */
