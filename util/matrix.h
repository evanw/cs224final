#ifndef MATRIX_H
#define MATRIX_H

#include <assert.h>
#include "vector.h"

class Matrix3
{
protected:
    float m_[3][3]; // [col][row]

public:
    Matrix3();
    Matrix3(const Matrix3&);

    Matrix3(float, float, float,
            float, float, float,
            float, float, float);

    // array access
    float* operator [](int);
    const float* operator [](int) const;

    // assignment
    Matrix3& assign(float, float, float,
                    float, float, float,
                    float, float, float);

    void setZero();
    void setIdentity();

    // math operators
    Matrix3& operator =(const Matrix3&);
    Matrix3& operator +=(const Matrix3&);
    Matrix3& operator -=(const Matrix3&);
    Matrix3& operator *=(const Matrix3&);
    Matrix3& operator *=(float);
    Matrix3& operator /=(float);

    Matrix3 operator +(const Matrix3&) const;
    Matrix3 operator -(const Matrix3&) const;
    Matrix3 operator -() const;
    Matrix3 operator *(const Matrix3&) const;
    Matrix3 operator *(float) const;
    Matrix3 operator /(float) const;

    friend Matrix3 operator *(float, const Matrix3&);

    bool operator ==(const Matrix3&) const;
    bool operator !=(const Matrix3&) const;

    Vector3 operator *(const Vector3&) const;
    friend Vector3 operator *(const Vector3&, const Matrix3&);

    // operations
    Matrix3 inverse() const;
    Matrix3 transpose() const;
    Matrix3 adjoint() const;
    float determinant() const;
    bool isSingular() const;

    // transformation matrices
    static Matrix3 identity();
    static Matrix3 rotate(float);
    static Matrix3 scale(float, float);
    static Matrix3 scaleUniform(float);
    static Matrix3 translate(float, float);

    void print( char *str );
};


inline Matrix3::Matrix3() {
    setZero();
}

inline Matrix3::Matrix3(const Matrix3& M) {
    (*this) = M;
}

inline Matrix3::Matrix3(float e00, float e01, float e02,
                        float e10, float e11, float e12,
                        float e20, float e21, float e22) {
    assign(e00, e01, e02,
           e10, e11, e12,
           e20, e21, e22);
}

inline float* Matrix3::operator [](int i) {
    assert(i == 0 || i == 1 || i == 2 || i == 3);
    return &m_[i][0];
}

inline const float* Matrix3::operator [](int i) const {
    assert(i == 0 || i == 1 || i == 2 || i == 3);
    return &m_[i][0];
}

inline Matrix3& Matrix3::assign(float e00, float e01, float e02,
                                float e10, float e11, float e12,
                                float e20, float e21, float e22) {
    m_[0][0] = e00; m_[1][0] = e01; m_[2][0] = e02;
    m_[0][1] = e10; m_[1][1] = e11; m_[2][1] = e12;
    m_[0][2] = e20; m_[1][2] = e21; m_[2][2] = e22;
    return *this;
}

inline void Matrix3::setZero() {
    assign(0,0,0,
           0,0,0,
           0,0,0);
}

inline void Matrix3::setIdentity() {
    assign(1,0,0,
           0,1,0,
           0,0,1);
}


inline Matrix3& Matrix3::operator =(const Matrix3& M) {
    assign(M[0][0], M[1][0], M[2][0],
           M[0][1], M[1][1], M[2][1],
           M[0][2], M[1][2], M[2][2]);
    return *this;
}

/* inline void Matrix3::copyFrom(float f[3][3]) {
    assign(f[0][0], f[1][0], f[2][0],
           f[0][1], f[1][1], f[2][1],
           f[0][2], f[1][2], f[2][2]);
}

void Matrix3::copyTo(float f[3][3]) const {
    f[0][0] = m_[0][0]; f[0][1] = m_[0][1]; f[0][2] = m_[0][2];
    f[1][0] = m_[1][0]; f[1][1] = m_[1][1]; f[1][2] = m_[1][2];
    f[2][0] = m_[2][0]; f[2][1] = m_[2][1]; f[2][2] = m_[2][2];
} */

#endif // MATRIX_H
