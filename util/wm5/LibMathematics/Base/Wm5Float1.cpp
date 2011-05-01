// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#include "Wm5MathematicsPCH.h"
#include "Wm5Float1.h"
using namespace Wm5;

//----------------------------------------------------------------------------
Float1::Float1 ()
{
}
//----------------------------------------------------------------------------
Float1::~Float1 ()
{
}
//----------------------------------------------------------------------------
Float1::Float1 (float f0)
{
    mTuple[0] = f0;
}
//----------------------------------------------------------------------------
Float1::Float1 (const Float1& tuple)
{
    mTuple[0] = tuple.mTuple[0];
}
//----------------------------------------------------------------------------
Float1& Float1::operator= (const Float1& tuple)
{
    mTuple[0] = tuple.mTuple[0];
    return *this;
}
//----------------------------------------------------------------------------
