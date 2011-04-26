/********************************************************************
 *
 *   +-------------+
 *  / Description /
 * +-------------+
 *
 * This code computes the 3D convex hull of a points set.
 *
 * It is based on the code from "Computational Geometry in C"
 * (Second Edition) by Joseph O'Rourke.
 * The original code can be found on the following website
 * http://cs.smith.edu/~orourke/books/ftp.html
 *
 * This adaptation provides a class Chull3D which computes the
 * 3D convex hull of a set of vertices.
 *
 *   +-------+
 *  / Input /
 * +-------+
 *
 * float *vertices;
 * int n_vertices
 * vertices are organized as follows:
 * i-th vertex   : vertices[3*i], vertices[3*i+1], vertices[3*i+2]
 * so, the size of vertices is 3*n_vertices
 *
 *   +--------+
 *  / Output /
 * +--------+
 *
 * after callingthe method "compute", the convex hull is organized
 * as follows:
 * vertices (from the class Chull3D) contains the vertices of the
 * hull. faces contains the faces defining the hull.
 * The result can be sent in a OBJ file (export_obj) or into arrays
 * for visualization.
 *
 ********************************************************************/
#ifndef __CHULL3D_H__
#define __CHULL3D_H__

#include "document.h"

class Chull3D_vertex;
class Chull3D_edge;
class Chull3D_face;

/***************/
/*** Chull3D ***/
/***************/
class Chull3D
{
  friend class Chull3D_edge;
  friend class Chull3D_face;
 public:
  Chull3D (float *vertices, int n_vertices);
  ~Chull3D ();

  void compute        (void);

  int  get_n_vertices (void);
  int  get_n_faces    (void);

  /* output */
  int get_convex_hull (float **vertices, int *n_vertices, int **faces, int *n_faces);
  void export_mesh (Mesh &mesh);

 private:
  void add_vertex    (Chull3D_vertex *v);
  void add_edge      (Chull3D_edge *e);
  void add_face      (Chull3D_face *f);
  void delete_vertex (Chull3D_vertex *v);
  void delete_edge   (Chull3D_edge *e);
  void delete_face   (Chull3D_face *f);

  int are_collinear   (Chull3D_vertex *v1, Chull3D_vertex *v2, Chull3D_vertex *v3);
  int volume_sign     (Chull3D_face *f, Chull3D_vertex *v);
  int add_one         (Chull3D_vertex *v);
  void clean_up       (Chull3D_vertex *vnext);
  void clean_edges    (void);
  void clean_faces    (void);
  void clean_vertices (Chull3D_vertex *vnext);
  int double_triangle (void);
  int construct_hull  (void);

  int get_vertex_index (Chull3D_vertex *v);

 private:
  Chull3D_vertex *vertices;
  Chull3D_edge *edges;
  Chull3D_face *faces;
};

/**********************/
/*** Chull3D_vertex ***/
/**********************/
class Chull3D_vertex
{
  friend class Chull3D;
  friend class Chull3D_face;
 public:
  Chull3D_vertex () {};
  Chull3D_vertex (float x, float y, float z);

 private:
  float          pt[3];
  Chull3D_edge   *duplicate;
  short          on_hull;
  short          processed;
  Chull3D_vertex *prev, *next;
};

/********************/
/*** Chull3D_edge ***/
/********************/
class Chull3D_edge
{
  friend class Chull3D;
  friend class Chull3D_face;
 public:
  Chull3D_edge (Chull3D *hull3D);

 private:
  Chull3D_face   *adj_faces[2];
  Chull3D_vertex *end_points[2];
  Chull3D_face   *new_face;
  int            to_delete;
  Chull3D_edge   *prev, *next;
};

/********************/
/*** Chull3D_face ***/
/********************/
class Chull3D_face
{
  friend class Chull3D;
 public:
  Chull3D_face (Chull3D *hull3D, Chull3D_vertex *v1, Chull3D_vertex *v2, Chull3D_vertex *v3, Chull3D_face *f);
  Chull3D_face (Chull3D *hull3D, Chull3D_edge *e, Chull3D_vertex *v);

 private:
  void make_ccw (Chull3D_edge *e, Chull3D_vertex *v);

 private:
  Chull3D_edge   *edges[3];
  Chull3D_vertex *vertices[3];
  int            visible;
  Chull3D_face   *prev, *next;
};

#endif /* __CHULL3D_H__ */

