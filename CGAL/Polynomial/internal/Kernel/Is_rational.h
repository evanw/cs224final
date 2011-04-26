// Copyright (c) 2005  Stanford University (USA).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Kinetic_data_structures/include/CGAL/Polynomial/internal/Kernel/Is_rational.h $
// $Id: Is_rational.h 56668 2010-06-09 08:45:58Z sloriot $
// 
//
// Author(s)     : Daniel Russel <drussel@alumni.princeton.edu>

#ifndef CGAL_POLYNOMIAL_INTERNAL_IS_RATIONAL_H
#define CGAL_POLYNOMIAL_INTERNAL_IS_RATIONAL_H

#include <CGAL/Polynomial/basic.h>

namespace CGAL { namespace POLYNOMIAL { namespace internal {

//! Compute the sign after a root.
/*!
  This has specializations for Explicit_roots.
*/
template <class K>
class Is_rational
{
public:
  Is_rational(){  }

  typedef bool result_type;
  typedef typename K::Root argument_type;

  template <class T>
  result_type operator()(const T &v) const
  {
    return v.is_rational();
  }

  bool operator()(double) const
  {
    return true;
  }
#ifdef CGAL_USE_CORE
  bool operator()(const CORE::Expr) const
  {
    return false;
  }
#endif
};

} } } //namespace CGAL::POLYNOMIAL::internal
#endif
