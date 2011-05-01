// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2011/03/27)

//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>::GMatrix (int rows, int columns)
{
    mData = 0;
    mEntry = 0;
    SetSize(rows, columns);
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>::GMatrix (int rows, int columns, const Real* entry)
{
    mData = 0;
    mEntry = 0;
    SetMatrix(rows, columns, entry);
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>::GMatrix (int rows, int columns, const Real** matrix)
{
    mData = 0;
    mEntry = 0;
    SetMatrix(rows, columns, matrix);
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>::GMatrix (const GMatrix& mat)
{
    mRows = 0;
    mColumns = 0;
    mQuantity = 0;
    mData = 0;
    mEntry = 0;
    *this = mat;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>::~GMatrix ()
{
    Deallocate();
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::SetSize (int rows, int columns)
{
    Deallocate();
    if (rows > 0 && columns > 0)
    {
        mRows = rows;
        mColumns = columns;
        mQuantity = mRows*mColumns;
        Allocate(true);
    }
    else
    {
        mRows = 0;
        mColumns = 0;
        mQuantity = 0;
        mData = 0;
        mEntry = 0;
    }
}
//----------------------------------------------------------------------------
template <typename Real>
inline void GMatrix<Real>::GetSize (int& rows, int& columns) const
{
    rows = mRows;
    columns = mColumns;
}
//----------------------------------------------------------------------------
template <typename Real>
inline int GMatrix<Real>::GetRows () const
{
    return mRows;
}
//----------------------------------------------------------------------------
template <typename Real>
inline int GMatrix<Real>::GetColumns () const
{
    return mColumns;
}
//----------------------------------------------------------------------------
template <typename Real>
inline int GMatrix<Real>::GetQuantity () const
{
    return mQuantity;
}
//----------------------------------------------------------------------------
template <typename Real>
inline GMatrix<Real>::operator const Real* () const
{
    return mData;
}
//----------------------------------------------------------------------------
template <typename Real>
inline GMatrix<Real>::operator Real* ()
{
    return mData;
}
//----------------------------------------------------------------------------
template <typename Real>
inline const Real* GMatrix<Real>::operator[] (int row) const
{
    assertion(0 <= row && row < mRows, "Invalid index in operator[]\n");
    return mEntry[row];
}
//----------------------------------------------------------------------------
template <typename Real>
inline Real* GMatrix<Real>::operator[] (int row)
{
    assertion(0 <= row && row < mRows, "Invalid index in operator[]\n");
    return mEntry[row];
}
//----------------------------------------------------------------------------
template <typename Real>
inline const Real& GMatrix<Real>::operator() (int row, int column) const
{
    return mEntry[row][column];
}
//----------------------------------------------------------------------------
template <typename Real>
inline Real& GMatrix<Real>::operator() (int row, int column)
{
    assertion(0 <= row && row < mRows && 0 <= column &&
        column <= mColumns, "Invalid index in operator()\n");

    return mEntry[row][column];
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::SwapRows (int row0, int row1)
{
    assertion(0 <= row0 && row0 < mRows && 0 <= row1 && row1 < mRows,
        "Invalid index in SwapRows\n");

    Real* save = mEntry[row0];
    mEntry[row0] = mEntry[row1];
    mEntry[row1] = save;
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::SetRow (int row, const GVector<Real>& vec)
{
    assertion(0 <= row && row < mRows && vec.GetSize() == mColumns,
        "Invalid index in SetRow\n");

    for (int c = 0; c < mColumns; ++c)
    {
        mEntry[row][c] = vec[c];
    }
}
//----------------------------------------------------------------------------
template <typename Real>
GVector<Real> GMatrix<Real>::GetRow (int row) const
{
    assertion(0 <= row && row < mRows, "Invalid index in SetRow\n");

    GVector<Real> vec(mColumns);
    for (int c = 0; c < mColumns; ++c)
    {
        vec[c] = mEntry[row][c];
    }
    return vec;
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::SetColumn (int column, const GVector<Real>& vec)
{
    assertion(0 <= column && column < mColumns && vec.GetSize() == mRows,
        "Invalid index in SetColumn\n");

    for (int r = 0; r < mRows; ++r)
    {
        mEntry[r][column] = vec[r];
    }
}
//----------------------------------------------------------------------------
template <typename Real>
GVector<Real> GMatrix<Real>::GetColumn (int column) const
{
    assertion(0 <= column && column < mColumns,
        "Invalid index in GetColumn\n");

    GVector<Real> vec(mRows);
    for (int r = 0; r < mRows; ++r)
    {
        vec[r] = mEntry[r][column];
    }
    return vec;
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::SetMatrix (int rows, int columns, const Real* entry)
{
    Deallocate();
    if (rows > 0 && columns > 0)
    {
        mRows = rows;
        mColumns = columns;
        mQuantity = mRows*mColumns;
        Allocate(false);
        size_t numBytes = mQuantity*sizeof(Real);
        memcpy(mData, entry, numBytes);
    }
    else
    {
        mRows = 0;
        mColumns = 0;
        mQuantity = 0;
        mData = 0;
        mEntry = 0;
    }
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::SetMatrix (int rows, int columns, const Real** matrix)
{
    Deallocate();
    if (rows > 0 && columns > 0)
    {
        mRows = rows;
        mColumns = columns;
        mQuantity = mRows*mColumns;
        Allocate(false);
        for (int r = 0; r < mRows; ++r)
        {
            for (int c = 0; c < mColumns; ++c)
            {
                mEntry[r][c] = matrix[r][c];
            }
        }
    }
    else
    {
        mRows = 0;
        mColumns = 0;
        mQuantity = 0;
        mData = 0;
        mEntry = 0;
    }
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::GetColumnMajor (Real* columnMajor) const
{
    for (int r = 0, i = 0; r < mRows; ++r)
    {
        for (int c = 0; c < mColumns; ++c, ++i)
        {
            columnMajor[i] = mEntry[c][r];
        }
    }
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>& GMatrix<Real>::operator= (const GMatrix& mat)
{
    if (mat.mQuantity > 0)
    {
        if (mRows != mat.mRows || mColumns != mat.mColumns)
        {
            Deallocate();
            mRows = mat.mRows;
            mColumns = mat.mColumns;
            mQuantity = mat.mQuantity;
            Allocate(false);
        }
        for (int r = 0; r < mRows; ++r)
        {
            for (int c = 0; c < mColumns; ++c)
            {
                mEntry[r][c] = mat.mEntry[r][c];
            }
        }
    }
    else
    {
        Deallocate();
        mRows = 0;
        mColumns = 0;
        mQuantity = 0;
        mData = 0;
        mEntry = 0;
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
bool GMatrix<Real>::operator== (const GMatrix& mat) const
{
    return memcmp(mData, mat.mData, mQuantity*sizeof(Real)) == 0;
}
//----------------------------------------------------------------------------
template <typename Real>
bool GMatrix<Real>::operator!= (const GMatrix& mat) const
{
    return memcmp(mData, mat.mData, mQuantity*sizeof(Real)) != 0;
}
//----------------------------------------------------------------------------
template <typename Real>
bool GMatrix<Real>::operator<  (const GMatrix& mat) const
{
    return memcmp(mData, mat.mData, mQuantity*sizeof(Real)) < 0;
}
//----------------------------------------------------------------------------
template <typename Real>
bool GMatrix<Real>::operator<= (const GMatrix& mat) const
{
    return memcmp(mData, mat.mData, mQuantity*sizeof(Real)) <= 0;
}
//----------------------------------------------------------------------------
template <typename Real>
bool GMatrix<Real>::operator>  (const GMatrix& mat) const
{
    return memcmp(mData, mat.mData, mQuantity*sizeof(Real)) > 0;
}
//----------------------------------------------------------------------------
template <typename Real>
bool GMatrix<Real>::operator>= (const GMatrix& mat) const
{
    return memcmp(mData, mat.mData, mQuantity*sizeof(Real)) >= 0;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::operator+ (const GMatrix& mat) const
{
    GMatrix<Real> result(mat.mRows, mat.mColumns);
    for (int i = 0; i < mQuantity; ++i)
    {
        result.mData[i] = mData[i] + mat.mData[i];
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::operator- (const GMatrix& mat) const
{
    GMatrix<Real> result(mat.mRows, mat.mColumns);
    for (int i = 0; i < mQuantity; ++i)
    {
        result.mData[i] = mData[i] - mat.mData[i];
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::operator* (const GMatrix& mat) const
{
    // 'this' is RxN, 'M' is NxC, 'product = this*M' is RxC
    assertion(mColumns == mat.mRows, "Mismatch in operator*\n");

    GMatrix<Real> result(mRows, mat.mColumns);
    for (int r = 0; r < result.mRows; ++r)
    {
        for (int c = 0; c < result.mColumns; ++c)
        {
            for (int m = 0; m < mColumns; ++m)
            {
                result.mEntry[r][c] += mEntry[r][m] * mat.mEntry[m][c];
            }
        }
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::operator* (Real scalar) const
{
    GMatrix<Real> result(mRows, mColumns);
    for (int i = 0; i < mQuantity; ++i)
    {
        result.mData[i] = scalar*mData[i];
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::operator/ (Real scalar) const
{
    GMatrix<Real> result(mRows, mColumns);
    int i;

    if (scalar != (Real)0)
    {
        Real invScalar = ((Real)1)/scalar;
        for (i = 0; i < mQuantity; ++i)
        {
            result.mData[i] = invScalar*mData[i];
        }
    }
    else
    {
        for (i = 0; i < mQuantity; ++i)
        {
            result.mData[i] = Math<Real>::MAX_REAL;
        }
    }

    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::operator- () const
{
    GMatrix<Real> result(mRows, mColumns);
    for (int i = 0; i < mQuantity; ++i)
    {
        result.mData[i] = -mData[i];
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>& GMatrix<Real>::operator+= (const GMatrix& mat)
{
    for (int i = 0; i < mQuantity; ++i)
    {
        mData[i] += mat.mData[i];
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>& GMatrix<Real>::operator-= (const GMatrix& mat)
{
    for (int i = 0; i < mQuantity; ++i)
    {
        mData[i] -= mat.mData[i];
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>& GMatrix<Real>::operator*= (Real scalar)
{
    for (int i = 0; i < mQuantity; ++i)
    {
        mData[i] *= scalar;
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real>& GMatrix<Real>::operator/= (Real scalar)
{
    int i;

    if (scalar != (Real)0)
    {
        Real invScalar = ((Real)1)/scalar;
        for (i = 0; i < mQuantity; ++i)
        {
            mData[i] *= invScalar;
        }
    }
    else
    {
        for (i = 0; i < mQuantity; ++i)
        {
            mData[i] = Math<Real>::MAX_REAL;
        }
    }

    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
GVector<Real> GMatrix<Real>::operator* (const GVector<Real>& vec) const
{
    assertion(vec.GetSize() == mColumns, "Mismatch in operator*\n");

    GVector<Real> result(mRows);
    for (int r = 0; r < mRows; ++r)
    {
        for (int c = 0; c < mColumns; ++c)
        {
            result[r] += mEntry[r][c]*vec[c];
        }
            
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
Real GMatrix<Real>::QForm (const GVector<Real>& u, const GVector<Real>& v)
    const
{
    assertion(u.GetSize() == mRows && v.GetSize() == mColumns,
        "Invalid index in QForm\n");

    return u.Dot((*this)*v);
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::Transpose () const
{
    GMatrix<Real> result(mColumns, mRows);
    for (int r = 0; r < mRows; ++r)
    {
        for (int c = 0; c < mColumns; ++c)
        {
            result.mEntry[c][r] = mEntry[r][c];
        }
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::TransposeTimes (const GMatrix& mat) const
{
    // P = A^T*B, P[r][c] = sum_m A[m][r]*B[m][c]
    assertion(mRows == mat.mRows, "Mismatch in TransposeTimes\n");

    GMatrix<Real> result(mColumns, mat.mColumns);
    for (int r = 0; r < result.mRows; ++r)
    {
        for (int c = 0; c < result.mColumns; ++c)
        {
            for (int m = 0; m < mRows; ++m)
            {
                result.mEntry[r][c] += mEntry[m][r] * mat.mEntry[m][c];
            }
        }
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::TimesTranspose (const GMatrix& mat) const
{
    // P = A*B^T, P[r][c] = sum_m A[r][m]*B[c][m]
    assertion(mColumns == mat.mColumns, "Mismatch in TimesTranspose\n");

    GMatrix<Real> result(mRows, mat.mRows);
    for (int r = 0; r < result.mRows; ++r)
    {
        for (int c = 0; c < result.mColumns; ++c)
        {
            for (int m = 0; m < mColumns; ++m)
            {
                result.mEntry[r][c] +=  mEntry[r][m] * mat.mEntry[c][m];
            }
        }
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
GMatrix<Real> GMatrix<Real>::TransposeTimesTranspose (const GMatrix& mat)
    const
{
    // P = A*B^T, P[r][c] = sum_m A[m][r]*B[c][m]
    assertion(mColumns == mat.mColumns,
        "Mismatch in TransposeTimesTranspose\n");

    GMatrix<Real> result(mColumns, mat.mRows);
    for (int r = 0; r < result.mRows; ++r)
    {
        for (int c = 0; c < result.mColumns; ++c)
        {
            for (int m = 0; m < mColumns; ++m)
            {
                result.mEntry[r][c] +=  mEntry[m][r] * mat.mEntry[c][m];
            }
        }
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename Real>
bool GMatrix<Real>::GetInverse (GMatrix<Real>& inverse) const
{
    // Computations are performed in-place.
    if (GetRows() > 0 && GetRows() != GetColumns())
    {
        return false;
    }

    int size = GetRows();
    inverse = *this;

    int* colIndex = new1<int>(size);
    int* rowIndex = new1<int>(size);
    bool* pivoted = new1<bool>(size);
    memset(pivoted, 0, size*sizeof(bool));

    int i1, i2, row = 0, col = 0;
    Real save;

    // Elimination by full pivoting.
    for (int i0 = 0; i0 < size; ++i0)
    {
        // Search matrix (excluding pivoted rows) for maximum absolute entry.
        Real max = (Real)0;
        for (i1 = 0; i1 < size; ++i1)
        {
            if (!pivoted[i1])
            {
                for (i2 = 0; i2 < size; ++i2)
                {
                    if (!pivoted[i2])
                    {
                        Real abs = Math<Real>::FAbs(inverse[i1][i2]);
                        if (abs > max)
                        {
                            max = abs;
                            row = i1;
                            col = i2;
                        }
                    }
                }
            }
        }

        if (max == (Real)0)
        {
            // Matrix is not invertible.
            delete1(colIndex);
            delete1(rowIndex);
            delete1(pivoted);
            return false;
        }

        pivoted[col] = true;

        // Swap rows so that A[col][col] contains the pivot entry.
        if (row != col)
        {
            inverse.SwapRows(row, col);
        }

        // Keep track of the permutations of the rows.
        rowIndex[i0] = row;
        colIndex[i0] = col;

        // Scale the row so that the pivot entry is 1.
        Real inv = ((Real)1)/inverse[col][col];
        inverse[col][col] = (Real)1;
        for (i2 = 0; i2 < size; ++i2)
        {
            inverse[col][i2] *= inv;
        }

        // Zero out the pivot column locations in the other rows.
        for (i1 = 0; i1 < size; ++i1)
        {
            if (i1 != col)
            {
                save = inverse[i1][col];
                inverse[i1][col] = (Real)0;
                for (i2 = 0; i2 < size; ++i2)
                {
                    inverse[i1][i2] -= inverse[col][i2]*save;
                }
            }
        }
    }

    // Reorder rows so that A[][] stores the inverse of the original matrix.
    for (i1 = size-1; i1 >= 0; --i1)
    {
        if (rowIndex[i1] != colIndex[i1])
        {
            for (i2 = 0; i2 < size; ++i2)
            {
                save = inverse[i2][rowIndex[i1]];
                inverse[i2][rowIndex[i1]] = inverse[i2][colIndex[i1]];
                inverse[i2][colIndex[i1]] = save;
            }
        }
    }

    delete1(colIndex);
    delete1(rowIndex);
    delete1(pivoted);
    return true;
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::Allocate (bool setToZero)
{
    // assert:  mRows, mColumns, and mQuantity already initialized

    mData = new1<Real>(mQuantity);
    if (setToZero)
    {
        memset(mData, 0, mQuantity*sizeof(Real));
    }

    mEntry = new1<Real*>(mRows);
    for (int r = 0; r < mRows; ++r)
    {
        mEntry[r] = &mData[r*mColumns];
    }
}
//----------------------------------------------------------------------------
template <typename Real>
void GMatrix<Real>::Deallocate ()
{
    delete1(mData);
    delete1(mEntry);
}
//----------------------------------------------------------------------------
