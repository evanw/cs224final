// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/10/01)

#include "Wm5MathematicsPCH.h"
#include "Wm5ConvexHull.h"
#include "Wm5Memory.h"

namespace Wm5
{
//----------------------------------------------------------------------------
template <typename Real>
ConvexHull<Real>::ConvexHull (int numVertices, Real epsilon, bool owner,
    Query::Type queryType)
    :
    mQueryType(queryType),
    mNumVertices(numVertices),
    mDimension(0),
    mNumSimplices(0),
    mIndices(0),
    mEpsilon(epsilon),
    mOwner(owner)
{
    assertion(mNumVertices > 0 && mEpsilon >= (Real)0, "Invalid inputs\n");
}
//----------------------------------------------------------------------------
template <typename Real>
ConvexHull<Real>::~ConvexHull ()
{
    delete1(mIndices);
}
//----------------------------------------------------------------------------
template <typename Real>
int ConvexHull<Real>::GetQueryType () const
{
    return mQueryType;
}
//----------------------------------------------------------------------------
template <typename Real>
int ConvexHull<Real>::GetNumVertices () const
{
    return mNumVertices;
}
//----------------------------------------------------------------------------
template <typename Real>
Real ConvexHull<Real>::GetEpsilon () const
{
    return mEpsilon;
}
//----------------------------------------------------------------------------
template <typename Real>
bool ConvexHull<Real>::GetOwner () const
{
    return mOwner;
}
//----------------------------------------------------------------------------
template <typename Real>
int ConvexHull<Real>::GetDimension () const
{
    return mDimension;
}
//----------------------------------------------------------------------------
template <typename Real>
int ConvexHull<Real>::GetNumSimplices () const
{
    return mNumSimplices;
}
//----------------------------------------------------------------------------
template <typename Real>
const int* ConvexHull<Real>::GetIndices () const
{
    return mIndices;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Explicit instantiation.
//----------------------------------------------------------------------------
template WM5_MATHEMATICS_ITEM
class ConvexHull<float>;

template WM5_MATHEMATICS_ITEM
class ConvexHull<double>;
//----------------------------------------------------------------------------
}
