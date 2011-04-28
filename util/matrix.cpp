#include "matrix.h"
//
// private function: RCD
// - dot product of row i of matrix A and row j of matrix B
//
inline float RCD(const Matrix3& A, const Matrix3& B, int i, int j) {
    return A[i][0] * B[0][j] + A[i][1] * B[1][j] + A[i][2] * B[2][j];
}

//
// private function: MINOR
// - MINOR of M[r][c]; r in {0,1,2}-{r0,r1}; c in {0,1,2}-{c0,c1}
//
inline float MINOR(const Matrix3& M, int r0, int r1, int c0, int c1) {
    return M[r0][c0] * M[r1][c1] - M[r1][c0] * M[r0][c1];
}


inline bool fuzzyEQ(float f1, float f2) {
    return fabs(f1 - f2) < 1e-8;
}

Matrix3& Matrix3::operator +=(const Matrix3& M) {
    m_[0][0] += M[0][0]; m_[0][1] += M[0][1]; m_[0][2] += M[0][2];
    m_[1][0] += M[1][0]; m_[1][1] += M[1][1]; m_[1][2] += M[1][2];
    m_[2][0] += M[2][0]; m_[2][1] += M[2][1]; m_[2][2] += M[2][2];
    return *this;
}

Matrix3& Matrix3::operator -=(const Matrix3& M) {
    m_[0][0] -= M[0][0]; m_[0][1] -= M[0][1]; m_[0][2] -= M[0][2];
    m_[1][0] -= M[1][0]; m_[1][1] -= M[1][1]; m_[1][2] -= M[1][2];
    m_[2][0] -= M[2][0]; m_[2][1] -= M[2][1]; m_[2][2] -= M[2][2];
    return *this;
}

Matrix3& Matrix3::operator *=(const Matrix3& M) {
    assign(RCD(*this, M, 0, 0), RCD(*this, M, 1, 0), RCD(*this, M, 2, 0),
           RCD(*this, M, 0, 1), RCD(*this, M, 1, 1), RCD(*this, M, 2, 1),
           RCD(*this, M, 0, 2), RCD(*this, M, 1, 2), RCD(*this, M, 2, 2));
    return *this;
}

Matrix3& Matrix3::operator *=(float d) {
    m_[0][0] *= d; m_[0][1] *= d; m_[0][2] *= d;
    m_[1][0] *= d; m_[1][1] *= d; m_[1][2] *= d;
    m_[2][0] *= d; m_[2][1] *= d; m_[2][2] *= d;
    return *this;
}

Matrix3& Matrix3::operator /=(float d) {
    float di = 1 / d;
    m_[0][0] *= di; m_[0][1] *= di; m_[0][2] *= di;
    m_[1][0] *= di; m_[1][1] *= di; m_[1][2] *= di;
    m_[2][0] *= di; m_[2][1] *= di; m_[2][2] *= di;
    return *this;
}

Matrix3 Matrix3::operator +(const Matrix3& M) const {
    return Matrix3(m_[0][0] + M[0][0], m_[1][0] + M[1][0], m_[2][0] + M[2][0],
                   m_[0][1] + M[0][1], m_[1][1] + M[1][1], m_[2][1] + M[2][1],
                   m_[0][2] + M[0][2], m_[1][2] + M[1][2], m_[2][2] + M[2][2]);
}

Matrix3 Matrix3::operator -(const Matrix3& M) const {
    return Matrix3(m_[0][0] - M[0][0], m_[1][0] - M[1][0], m_[2][0] - M[2][0],
                   m_[0][1] - M[0][1], m_[1][1] - M[1][1], m_[2][1] - M[2][1],
                   m_[0][2] - M[0][2], m_[1][2] - M[1][2], m_[2][2] - M[2][2]);
}

Matrix3 Matrix3::operator -() const {
    return Matrix3(-m_[0][0], -m_[1][0], -m_[2][0],
                   -m_[0][1], -m_[1][1], -m_[2][1],
                   -m_[0][2], -m_[1][2], -m_[2][2]);
}

Matrix3 Matrix3::operator *(const Matrix3& M) const {
    return
            Matrix3(RCD(*this, M, 0, 0), RCD(*this, M, 1, 0), RCD(*this, M, 2, 0),
                    RCD(*this, M, 0, 1), RCD(*this, M, 1, 1), RCD(*this, M, 2, 1),
                    RCD(*this, M, 0, 2), RCD(*this, M, 1, 2), RCD(*this, M, 2, 2));
}

Matrix3 Matrix3::operator *(float d) const {
    return Matrix3(m_[0][0] * d, m_[1][0] * d, m_[2][0] * d,
                   m_[0][1] * d, m_[1][1] * d, m_[2][1] * d,
                   m_[0][2] * d, m_[1][2] * d, m_[2][2] * d);
}

Matrix3 Matrix3::operator /(float d) const {
    assert(!fuzzyEQ(d, 0));
    float di = 1 / d;
    return Matrix3(m_[0][0] * di, m_[1][0] * di, m_[2][0] * di,
                   m_[0][1] * di, m_[1][1] * di, m_[2][1] * di,
                   m_[0][2] * di, m_[1][2] * di, m_[2][2] * di);
}

Matrix3 operator *(float d, const Matrix3& M) {
    return Matrix3(M[0][0] * d, M[1][0] * d, M[2][0] * d,
                   M[0][1] * d, M[1][1] * d, M[2][1] * d,
                   M[0][2] * d, M[1][2] * d, M[2][2] * d);
}

bool Matrix3::operator ==(const Matrix3& M) const {
    return(fuzzyEQ(m_[0][0], M[0][0]) &&
           fuzzyEQ(m_[0][1], M[0][1]) &&
           fuzzyEQ(m_[0][2], M[0][2]) &&

           fuzzyEQ(m_[1][0], M[1][0]) &&
           fuzzyEQ(m_[1][1], M[1][1]) &&
           fuzzyEQ(m_[1][2], M[1][2]) &&

           fuzzyEQ(m_[2][0], M[2][0]) &&
           fuzzyEQ(m_[2][1], M[2][1]) &&
           fuzzyEQ(m_[2][2], M[2][2]));
}

bool Matrix3::operator !=(const Matrix3& M) const {
    return (!(*this == M));
}

Vector3 Matrix3::operator *(const Vector3& v) const {
    return Vector3(m_[0][0] * v.x + m_[0][1] * v.y + m_[0][2] * v.z,
                   m_[1][0] * v.x + m_[1][1] * v.y + m_[1][2] * v.z,
                   m_[2][0] * v.x + m_[2][1] * v.y + m_[2][2] * v.z);
}

Vector3 operator *(const Vector3& v, const Matrix3& M) {
    return Vector3(v.x * M[0][0] + v.y * M[1][0] + v.z * M[2][0],
                   v.x * M[0][1] + v.y * M[1][1] + v.z * M[2][1],
                   v.x * M[0][2] * v.y * M[1][2] + v.z * M[2][2]);
}

Matrix3 Matrix3::transpose() const {
    return Matrix3(m_[0][0], m_[0][1], m_[0][2],
                   m_[1][0], m_[1][1], m_[1][2],
                   m_[2][0], m_[2][1], m_[2][2]);
}

Matrix3 Matrix3::inverse() const {
    assert(!isSingular());
    return adjoint() / determinant();
}

Matrix3 Matrix3::adjoint() const {
    return Matrix3(
            MINOR(*this, 1, 2, 1, 2),
            -MINOR(*this, 1, 2, 0, 2),
            MINOR(*this, 1, 2, 0, 1),

            -MINOR(*this, 0, 2, 1, 2),
            MINOR(*this, 0, 2, 0, 2),
            -MINOR(*this, 0, 2, 0, 1),

            MINOR(*this, 0, 1, 1, 2),
            -MINOR(*this, 0, 1, 0, 2),
            MINOR(*this, 0, 1, 0, 1)
            );
}

float Matrix3::determinant() const {
    return m_[0][0] * MINOR(*this, 1, 2, 1, 2) -
            m_[0][1] * MINOR(*this, 1, 2, 0, 2) +
            m_[0][2] * MINOR(*this, 1, 2, 0, 1);
}

bool Matrix3::isSingular() const {
    return determinant();
}

Matrix3 Matrix3::identity() {
    return Matrix3(1, 0, 0,
                   0, 1, 0,
                   0, 0, 1);
}

Matrix3 Matrix3::rotate(float angle) {
    float sine = sinf(angle);
    float cosine = cosf(angle);

    return Matrix3(cosine, -sine,   0,
                   sine,    cosine, 0,
                   0,       0,      1);
}

Matrix3 Matrix3::scale(float sx, float sy) {
    return Matrix3(sx, 0,  0,
                   0,  sy, 0,
                   0,  0,  1);
}

Matrix3 Matrix3::scaleUniform(float s) {
    return Matrix3(s, 0, 0,
                   0, s, 0,
                   0, 0, 1);
}

Matrix3 Matrix3::translate(float x, float y) {
    return Matrix3(1, 0, x,
                   0, 1, y,
                   0, 0, 1);
}

void Matrix3::print( char *str ) {
    if( str ) {
        printf("%s\n", str );
    }
    printf(
            "%.2f %.2f %.2f\n"
            "%.2f %.2f %.2f\n"
            "%.2f %.2f %.2f\n",
            m_[0][0], m_[1][0], m_[2][0],
            m_[0][1], m_[1][1], m_[2][1],
            m_[0][2], m_[1][2], m_[2][2] );
}
