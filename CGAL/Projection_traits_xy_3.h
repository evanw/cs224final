// Copyright (c) 1997-2010  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Kernel_23/include/CGAL/Projection_traits_xy_3.h $
// $Id: Projection_traits_xy_3.h 60713 2011-01-11 07:18:36Z sloriot $
// 
//
// Author(s)     : Mariette Yvinec

#ifndef CGAL_PROJECTION_TRAITS_XY_3_H
#define CGAL_PROJECTION_TRAITS_XY_3_H

#include <CGAL/internal/Projection_traits_3.h>

namespace CGAL { 

template < class R >
class Projection_traits_xy_3
  : public internal::Projection_traits_3<R,2>
{};
  
} //namespace CGAL 

#endif // CGAL_PROJECTION_TRAITS_XY_3_H
