// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#include "Wm5MathematicsPCH.h"
#include "Wm5Float2.h"
using namespace Wm5;

//----------------------------------------------------------------------------
Float2::Float2 ()
{
}
//----------------------------------------------------------------------------
Float2::~Float2 ()
{
}
//----------------------------------------------------------------------------
Float2::Float2 (float f0, float f1)
{
    mTuple[0] = f0;
    mTuple[1] = f1;
}
//----------------------------------------------------------------------------
Float2::Float2 (const Float2& tuple)
{
    mTuple[0] = tuple.mTuple[0];
    mTuple[1] = tuple.mTuple[1];
}
//----------------------------------------------------------------------------
Float2& Float2::operator= (const Float2& tuple)
{
    mTuple[0] = tuple.mTuple[0];
    mTuple[1] = tuple.mTuple[1];
    return *this;
}
//----------------------------------------------------------------------------
