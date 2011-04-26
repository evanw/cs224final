// Copyright (c) 2006-2007  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Surface_mesher/include/CGAL/Surface_mesher/Has_edges.h $
// $Id: Has_edges.h 39009 2007-06-10 20:50:11Z lrineau $
// 
//
// Author(s)     : Laurent RINEAU

#ifndef CGAL_SURFACE_MESHER_HAS_EDGES_H
#define CGAL_SURFACE_MESHER_HAS_EDGES_H

namespace CGAL {
  namespace Surface_mesher {

    struct Has_edges {
      static bool has_edges() { return true; }
    };
    struct Has_no_edges {
      static bool has_edges() { return false; }
    };

  } // end namespace Surface_mesher
} // end namespace CGAL

#endif // CGAL_SURFACE_MESHER_HAS_EDGES_H
