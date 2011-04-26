// Copyright (c) 1997  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Alpha_shapes_3/include/CGAL/Weighted_alpha_shape_euclidean_traits_3.h $
// $Id: Weighted_alpha_shape_euclidean_traits_3.h 60236 2010-12-08 17:39:30Z sloriot $
// 
//
// Author(s)     : Tran Kai Frank DA <Frank.Da@sophia.inria.fr>

#ifndef CGAL_WEIGHTED_ALPHA_SHAPE_EUCLIDEAN_TRAITS_3_H
#define CGAL_WEIGHTED_ALPHA_SHAPE_EUCLIDEAN_TRAITS_3_H 

#include <CGAL/Regular_triangulation_euclidean_traits_3.h>

namespace CGAL {


   
//------------------ Traits class -------------------------------------

#ifdef CGAL_NO_DEPRECATED_CODE
#error The class Weighted_alpha_shape_euclidean_traits_3<K> is deprecated in favor of Regular_triangulation_euclidean_traits_3<K>.
#endif

template <class K>
class Weighted_alpha_shape_euclidean_traits_3 : public 
Regular_triangulation_euclidean_traits_3<K>
{};
  
  
} //namespace CGAL
  
#endif //CGAL_WEIGHTED_ALPHA_SHAPE_EUCLIDEAN_TRAITS_3_H 
