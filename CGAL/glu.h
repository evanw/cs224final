// Copyright (c) 2008 GeometryFactory, Sophia Antipolis (France)
//  All rights reserved.
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Installation/include/CGAL/glu.h $
// $Id: glu.h 46161 2008-10-08 11:31:10Z lrineau $

#ifndef CGAL_GLU_H
#define CGAL_GLU_H

#ifdef _MSC_VER
#  include <wtypes.h>
#  include <wingdi.h>
#endif

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#endif // CGAL_GLU_H
