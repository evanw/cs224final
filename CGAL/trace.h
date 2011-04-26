// Copyright (c) 2007-09  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/trunk/Point_set_processing_3/include/CGAL/trace.h $ 
// $Id: trace.h 51061 2009-08-05 11:05:54Z lsaboret $
//
// Author(s) : Laurent Saboret

#ifndef CGAL_TRACE_H
#define CGAL_TRACE_H

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>


// Trace utilities
// ---------------

// print_stderr() = printf-like function to print to stderr
inline void print_stderr(const char *fmt, ...)
{
  va_list argp;
  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  va_end(argp);
}

// CGAL_TRACE() = printf-like function to print to stderr
// if DEBUG_TRACE is defined (ignored otherwise)
#ifdef DEBUG_TRACE
  #define CGAL_TRACE  print_stderr
#else
  #define CGAL_TRACE  if (false) print_stderr
#endif

// CGAL_TRACE_STREAM = C++ stream that prints to std::cerr 
// if DEBUG_TRACE is defined (ignored otherwise)
#ifdef DEBUG_TRACE
  #define CGAL_TRACE_STREAM  std::cerr
#else
  #define CGAL_TRACE_STREAM  if (false) std::cerr
#endif


#endif // CGAL_TRACE_H
