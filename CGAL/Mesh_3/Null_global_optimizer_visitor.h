// Copyright (c) 2010 INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Mesh_3/include/CGAL/Mesh_3/Null_global_optimizer_visitor.h $
// $Id: Null_global_optimizer_visitor.h 57256 2010-07-01 08:27:03Z stayeb $
//
//
// Author(s)     : Stephane Tayeb
//
//******************************************************************************
// File Description : 
//******************************************************************************

#ifndef CGAL_MESH_3_NULL_GLOBAL_OPTIMIZER_VISITOR_H
#define CGAL_MESH_3_NULL_GLOBAL_OPTIMIZER_VISITOR_H

namespace CGAL {
namespace Mesh_3 {
  
template < typename C3T3 >
class Null_global_optimizer_visitor
{
  typedef typename C3T3::Triangulation    Tr;
  typedef typename Tr::Geom_traits::FT    FT;
  
public:
  void after_compute_moves() {}
  void after_move_points() {}
  void after_rebuild_restricted_delaunay() {}
  void end_of_iteration(int) {}
};

} // end namespace Mesh_3
} // end namespace CGAL

#endif // CGAL_MESH_3_NULL_GLOBAL_OPTIMIZER_VISITOR_H
