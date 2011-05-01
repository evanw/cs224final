// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2011/03/27)

#ifndef WM5GMATRIX_H
#define WM5GMATRIX_H

// Matrix operations are applied on the left.  For example, given a matrix M
// and a vector V, matrix-times-vector is M*V.  That is, V is treated as a
// column vector.  Some graphics APIs use V*M where V is treated as a row
// vector.  In this context the "M" matrix is really a transpose of the M as
// represented in Wild Magic.  Similarly, to apply two matrix operations M0
// and M1, in that order, you compute M1*M0 so that the transform of a vector
// is (M1*M0)*V = M1*(M0*V).  Some graphics APIs use M0*M1, but again these
// matrices are the transpose of those as represented in Wild Magic.  You
// must therefore be careful about how you interface the transformation code
// with graphics APIS.
//
// Matrices are stored in row-major order, matrix[row][col].

#include "Wm5MathematicsLIB.h"
#include "Wm5GVector.h"

namespace Wm5
{

template <typename Real>
class GMatrix
{
public:
    // Construction and destruction.
    GMatrix (int rows = 0, int columns = 0);
    GMatrix (int rows, int columns, const Real* entry);
    GMatrix (int rows, int columns, const Real** matrix);
    GMatrix (const GMatrix& mat);
    ~GMatrix ();

    // Coordinate access.
    void SetSize (int rows, int columns);
    inline void GetSize (int& rows, int& columns) const;
    inline int GetRows () const;
    inline int GetColumns () const;
    inline int GetQuantity () const;
    inline operator const Real* () const;
    inline operator Real* ();
    inline const Real* operator[] (int row) const;
    inline Real* operator[] (int row);
    inline const Real& operator() (int row, int column) const;
    inline Real& operator() (int row, int column);
    void SwapRows (int row0, int row1);
    void SetRow (int row, const GVector<Real>& vec);
    GVector<Real> GetRow (int row) const;
    void SetColumn (int column, const GVector<Real>& vec);
    GVector<Real> GetColumn (int column) const;
    void SetMatrix (int rows, int columns, const Real* entry);
    void SetMatrix (int rows, int columns, const Real** matrix);
    void GetColumnMajor (Real* columnMajor) const;

    // Assignment.
    GMatrix& operator= (const GMatrix& mat);

    // Comparison (for use by STL containers).
    bool operator== (const GMatrix& mat) const;
    bool operator!= (const GMatrix& mat) const;
    bool operator<  (const GMatrix& mat) const;
    bool operator<= (const GMatrix& mat) const;
    bool operator>  (const GMatrix& mat) const;
    bool operator>= (const GMatrix& mat) const;

    // Arithmetic operations.
    GMatrix operator+ (const GMatrix& mat) const;
    GMatrix operator- (const GMatrix& mat) const;
    GMatrix operator* (const GMatrix& mat) const;
    GMatrix operator* (Real fScalar) const;
    GMatrix operator/ (Real fScalar) const;
    GMatrix operator- () const;

    // Arithmetic updates.
    GMatrix& operator+= (const GMatrix& mat);
    GMatrix& operator-= (const GMatrix& mat);
    GMatrix& operator*= (Real fScalar);
    GMatrix& operator/= (Real fScalar);

    // M*vec
    GVector<Real> operator* (const GVector<Real>& vec) const;

    // u^T*M*v
    Real QForm (const GVector<Real>& u, const GVector<Real>& v) const;

    // M^T
    GMatrix Transpose () const;

    // M^T*mat
    GMatrix TransposeTimes (const GMatrix& mat) const;

    // M*mat^T
    GMatrix TimesTranspose (const GMatrix& mat) const;

    // M^T*mat^T
    GMatrix TransposeTimesTranspose (const GMatrix& mat) const;

    // Inversion.  The matrix must be square.  The function returns 'true'
    // whenever the matrix is square and invertible.
    bool GetInverse (GMatrix<Real>& inverse) const;

    // c * M
    friend GMatrix<Real> operator* (Real scalar, const GMatrix<Real>& mat)
    {
        return mat*scalar;
    }

    // v^T * M
    friend GVector<Real> operator* (const GVector<Real>& vec,
        const GMatrix<Real>& mat)
    {
        assertion(vec.GetSize() == mat.GetRows(), "Mismatch in operator*\n");
        GVector<Real> prod(mat.GetColumns());
        Real* entry = prod;
        for (int c = 0; c < mat.GetColumns(); ++c)
        {
            for (int r = 0; r < mat.GetRows(); ++r)
            {
                entry[c] += vec[r]*mat[r][c];
            }
        }
        return prod;
    }

protected:
    // Support for allocation and deallocation.  The allocation call requires
    // m_iRows, m_iCols, and m_iQuantity to have already been correctly
    // initialized.
    void Allocate (bool setToZero);
    void Deallocate ();

    int mRows, mColumns, mQuantity;

    // The matrix is stored in row-major form as a 1-dimensional array.
    Real* mData;

    // An array of pointers to the rows of the matrix.  The separation of
    // row pointers and actual data supports swapping of rows in linear
    // algebraic algorithms such as solving linear systems of equations.
    Real** mEntry;
};


#include "Wm5GMatrix.inl"

typedef GMatrix<float> GMatrixf;
typedef GMatrix<double> GMatrixd;

}

#endif
