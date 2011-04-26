#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "chull.h"

#define SWAP(t,x,y) { t = x; x = y; y = t; }
#define ADD( head, p )  if ( head )  { \
                                p->next = head; \
                                p->prev = head->prev; \
                                head->prev = p; \
                                p->prev->next = p; \
                        } \
                        else { \
                                head = p; \
                                head->next = head->prev = p; \
                        }

#define DELETE( head, p ) if ( head )  { \
                                if ( head == head->next ) \
                                        head = NULL;  \
                                else if ( p == head ) \
                                        head = head->next; \
                                p->next->prev = p->prev;  \
                                p->prev->next = p->next; \
                                delete p; \
                        }

Chull3D::Chull3D (float *v, int n)
{
  int i;
  assert (v);

  /* init vertices */
  vertices = NULL;
  for (i=0; i<n; i++)
    add_vertex (new Chull3D_vertex (v[3*i], v[3*i+1], v[3*i+2]));
  edges = NULL;
  faces = NULL;
}

Chull3D::~Chull3D ()
{
  Chull3D_vertex *v, *vt;
  Chull3D_edge *e, *et;
  Chull3D_face *f, *ft;

  // clean vertices
  v = vertices;
  do {
          vt = v;
          v = v->next;
          delete_vertex (vt);
  } while (vertices->next != vertices);
  delete_vertex (vertices);

  // clean edges
  e = edges;
  do {
          et = e;
          e = e->next;
          delete_edge (et);
  } while (edges->next != edges);
  delete_edge (edges);

  // clean faces
  f = faces;
  do {
          ft = f;
          f = f->next;
          delete_face (ft);
  } while (faces->next != faces);
  delete_face (faces);
}

void Chull3D::add_vertex    (Chull3D_vertex *v) { ADD (vertices, v); }
void Chull3D::add_edge      (Chull3D_edge *e)   { ADD (edges, e); }
void Chull3D::add_face      (Chull3D_face *f)   { ADD (faces, f); }
void Chull3D::delete_vertex (Chull3D_vertex *v) { DELETE (vertices, v); }
void Chull3D::delete_edge   (Chull3D_edge *e)   { DELETE (edges, e); }
void Chull3D::delete_face   (Chull3D_face *f)   { DELETE (faces, f); }

/**********/
/* output */
/**********/
int
Chull3D::get_n_vertices (void)
{
  Chull3D_vertex *v = vertices;
  int n=0;
  if (!vertices)
    return 0;
  else
    do { v = v->next; n++; } while (v != vertices);
  return n;
}

int
Chull3D::get_n_faces (void)
{
  Chull3D_face *f = faces;
  int n=0;
  if (!faces)
    return 0;
  else
    do { f = f->next; n++; } while (f != faces);
  return n;
}

int
Chull3D::get_vertex_index (Chull3D_vertex *v)
{
  Chull3D_vertex *v_walk = vertices;
  int index = 0;
  do {
    if (v_walk == v)
      return index;
    index++;
    v_walk = v_walk->next;
  } while (v_walk != vertices);
  return -1;
}

int
Chull3D::get_convex_hull (float **v, int *nv, int **f, int *nf)
{
  *nv = get_n_vertices ();
  *nf = get_n_faces ();

  /* memory allocation */
  float *cv_vertices = (float*)malloc(3*(*nv)*sizeof(float));
  int   *cv_faces    = (int*)malloc(3*(*nf)*sizeof(int));
  if (!cv_vertices || !cv_faces)
    {
      v = NULL; f = NULL;
      *nv = *nf = 0;
      return 0;
    }

  /* vertices */
  Chull3D_vertex *v_walk=vertices;
  int i=0;
  do {
    cv_vertices[3*i]   = v_walk->pt[0];
    cv_vertices[3*i+1] = v_walk->pt[1];
    cv_vertices[3*i+2] = v_walk->pt[2];
    v_walk = v_walk->next;
    i++;
  } while (v_walk != vertices);

  /* faces */
  i = 0;
  Chull3D_face *f_walk = faces;
  do {
    cv_faces[3*i]   = get_vertex_index (f_walk->vertices[0]);
    cv_faces[3*i+1] = get_vertex_index (f_walk->vertices[1]);
    cv_faces[3*i+2] = get_vertex_index (f_walk->vertices[2]);
    f_walk = f_walk->next;
    i++;
  } while (f_walk != faces);

  *v = cv_vertices;
  *f = cv_faces;

  return 1;
}

void
Chull3D::export_mesh (Mesh &mesh)
{
  mesh.vertices.clear();
  mesh.triangles.clear();
  mesh.quads.clear();

  /* vertices */
  Chull3D_vertex *v_walk = vertices;
  do {
    mesh.vertices += Vertex(Vector3(v_walk->pt[0], v_walk->pt[1], v_walk->pt[2]));
    v_walk = v_walk->next;
  } while (v_walk != vertices);

  /* faces */
  Chull3D_face *f_walk = faces;
  do {
      mesh.triangles += Triangle(
             get_vertex_index (f_walk->vertices[0]),
             get_vertex_index (f_walk->vertices[1]),
             get_vertex_index (f_walk->vertices[2]));
    f_walk = f_walk->next;
  } while (f_walk != faces);
}

/*****************/
/*** Algorithm ***/
/*****************/
void
Chull3D::compute (void)
{
  double_triangle ();
  construct_hull ();
}

/* builds the initial double triangle */
int
Chull3D::double_triangle (void)
{
  Chull3D_vertex *v0, *v1, *v2, *v3;
  int vol;

  /* find 3 non collinear points */
  v0 = vertices;
  while (are_collinear (v0, v0->next, v0->next->next))
    if ( (v0=v0->next) == vertices)
      {
        printf ("All the vertices are collinear\n");
        return 1;
      }
  v1 = v0->next;
  v2 = v1->next;

  /* mark the vertices as processed */
  v0->processed = 1;
  v1->processed = 1;
  v2->processed = 1;

  /* create the two "twins" faces */
  Chull3D_face *f0, *f1 = NULL;
  f0 = new Chull3D_face (this, v0, v1, v2, f1);
  f1 = new Chull3D_face (this, v2, v1, v0, f0);

  /* link adjacent face fields */
  f0->edges[0]->adj_faces[1] = f1;
  f0->edges[1]->adj_faces[1] = f1;
  f0->edges[2]->adj_faces[1] = f1;
  f1->edges[0]->adj_faces[1] = f0;
  f1->edges[1]->adj_faces[1] = f0;
  f1->edges[2]->adj_faces[1] = f0;

  /* find a fourth, non coplanar point to form tetrahedron */
  v3 = v2->next;
  vol = volume_sign (f0, v3);
  while (!vol)
    {
      if ( (v3=v3->next) == v0)
        {
          printf ("All the vertices are coplanar\n");
          return 1;
        }
      vol = volume_sign (f0, v3);
    }

  /* insure that v3 will be the first added */
  vertices = v3;

  return 0;
}

/*
 * construct_hull adds the vertices to the hull one at a time.
 */
int
Chull3D::construct_hull (void)
{
  Chull3D_vertex *v, *vnext;

  v = vertices;
  do {
    vnext = v->next;
    if (!v->processed)
      {
        v->processed = 1;
        add_one (v);
        clean_up (vnext);
      }
    v = vnext;
  } while (v != vertices);

  return 0;
}

/*
 * are_collinear checks to see if the three points given are
 * collinear by checking to see if each element of the cross
 * product is zero.
 */
int
Chull3D::are_collinear (Chull3D_vertex *v1, Chull3D_vertex *v2, Chull3D_vertex *v3)
{
  return
    (( v3->pt[2] - v1->pt[2] ) * ( v2->pt[1] - v1->pt[1] ) -
     ( v2->pt[2] - v1->pt[2] ) * ( v3->pt[1] - v1->pt[1] ) == 0
     && ( v2->pt[2] - v1->pt[2] ) * ( v3->pt[0] - v1->pt[0] ) -
     ( v2->pt[0] - v1->pt[0] ) * ( v3->pt[2] - v1->pt[2] ) == 0
     && ( v2->pt[0] - v1->pt[0] ) * ( v3->pt[1] - v1->pt[1] ) -
     ( v2->pt[1] - v1->pt[1] ) * ( v3->pt[0] - v1->pt[0] ) == 0);
}

/*
 * volume_sign returns the sign of the volume of the tetrahedron
 * determined by f and p.
 * Volume_sign is +1 iff p is on the negative side of f, where the
 * positive side is determined by the rh-rule. So the volume is
 * positive if the ccw normal to f points outside the tetrahedron.
 * The final fewer-multiplications form is due to Bob Williamson.
 */
int
Chull3D::volume_sign (Chull3D_face *f, Chull3D_vertex *v)
{
  float ax, ay, az, bx, by, bz, cx, cy, cz;
  float vol;

   ax = f->vertices[0]->pt[0] - v->pt[0];
   ay = f->vertices[0]->pt[1] - v->pt[1];
   az = f->vertices[0]->pt[2] - v->pt[2];
   bx = f->vertices[1]->pt[0] - v->pt[0];
   by = f->vertices[1]->pt[1] - v->pt[1];
   bz = f->vertices[1]->pt[2] - v->pt[2];
   cx = f->vertices[2]->pt[0] - v->pt[0];
   cy = f->vertices[2]->pt[1] - v->pt[1];
   cz = f->vertices[2]->pt[2] - v->pt[2];

   vol =   ax * (by*cz - bz*cy)
         + ay * (bz*cx - bx*cz)
         + az * (bx*cy - by*cx);

   if      ( vol >  0.0 ) return  1;
   else if ( vol <  0.0 ) return -1;
   else                   return  0;
}

/*
 * add_one is passed a vertex. It first determines all faces visible
 * from that point. If none are visible then the point is marked as
 * not on hull. Next is a loop over edges. If both faces adjacent to
 * an edge are visible, then the edge is marked for deletion. If just
 * one of the adjacent faces is visible then a new face is constructed.
 */
int
Chull3D::add_one (Chull3D_vertex *v)
{
  Chull3D_face *f;
  Chull3D_edge *e, *temp;
  int vol;
  int vis = 0;

  /* marks faces visible from v */
  f = faces;
  do {
    vol = volume_sign (f, v);
    if (vol<0)
      {
        f->visible = 1;
        vis = 1;
      }
    f = f->next;
  } while (f != faces);

  /* if no faces are visible from v, then v is inside the hull */
  if (!vis)
    {
      v->on_hull = 0;
      return 0;
    }

  /* mark edges in interior of visible region for deletion.
     erect a new face based on each border edge */
  e = edges;
  do {
    temp = e->next;
    if (e->adj_faces[0]->visible && e->adj_faces[1]->visible)
      /* e interior: mark for deletion */
      e->to_delete = 1;
    else if (e->adj_faces[0]->visible || e->adj_faces[1]->visible)
      /* e border: make a new face */
      e->new_face = new Chull3D_face (this, e, v);
    e = temp;
  } while (e != edges);

  return 1;
}

/*
 * goes through each data structure list and clears all flags
 * and NULLs out some pointers.
 */
void
Chull3D::clean_up (Chull3D_vertex *vnext)
{
  clean_edges ();
  clean_faces ();
  clean_vertices (vnext);
}

/*
 * runs through the edge list and cleans up the structure.
 * If there is a newface then it will put that face in place
 * of the visible face and NULL out newface. It also deletes
 * so marked edges.
 */
void
Chull3D::clean_edges (void)
{
  Chull3D_edge *e, *t;

  /* integrate the new face's into the data structure */
  /* check every edge */
  e = edges;
  do {
      if (e->new_face)
        {
          if (e->adj_faces[0]->visible) e->adj_faces[0] = e->new_face;
          else                          e->adj_faces[1] = e->new_face;
          e->new_face = NULL;
        }
      e = e->next;
  } while (e != edges);

  /* delete any edges marked for deletion */
  while (edges && edges->to_delete)
    {
      e = edges;
      delete_edge (e);
    }
  e = edges->next;
  do {
    if (e->to_delete)
      {
        t = e;
        e = e->next;
        delete_edge (t);
      }
    else
      e = e->next;
  } while (e != edges);
}

/*
 * runs through the face list and deletes any face marked visible.
 */
void
Chull3D::clean_faces (void)
{
  Chull3D_face *f, *t;

  while (faces && faces->visible)
    {
      f = faces;
      delete_face (f);
    }
  f = faces->next;
  do {
    if (f->visible)
      {
        t = f;
        f = f->next;
        delete_face (t);
      }
    else
      f = f->next;
  } while (f != faces);
}

/*
 * runs through the vertex list and deletes the vertice
 * that are marked as processed but are not incident to
 * any undeleted edges.
 * The pointer to vnext, pvnext, is used to alter vnext
 * in construct_hull() if we are about to delete vnext.
*/
void
Chull3D::clean_vertices (Chull3D_vertex *vnext)
{
  Chull3D_edge *e;
  Chull3D_vertex *v, *t;

  /* mark all vertices incident to some undeleted edge as
     on the hull */
  e = edges;
  do {
    e->end_points[0]->on_hull = e->end_points[1]->on_hull = 1;
    e = e->next;
  } while (e != edges);

  /* delete all vertices that have been processed but are
     not on the hull */
  while (vertices && vertices->processed && !vertices->on_hull)
    {
      /* if about to delete vnext, advance it first */
      if (v == vnext)
        vnext = v->next;
        v = vertices;
        delete_vertex (v);
    }
  v = vertices->next;
  do {
    if (v->processed && !v->on_hull)
      {
        t = v;
        v = v->next;
        delete_vertex (t);
      }
    else
      v = v->next;
  } while (v != vertices);

  /* reset flags */
  v = vertices;
  do {
      v->duplicate = NULL;
      v->on_hull = 0;
      v = v->next;
  } while (v != vertices);
}

/**********************/
/*** Chull3D_vertex ***/
/**********************/
Chull3D_vertex::Chull3D_vertex (float x, float y, float z)
{
  pt[0] = x;  pt[1] = y;  pt[2] = z;
  duplicate = NULL;
  on_hull   = 0;
  processed = 0;
}

/********************/
/*** Chull3D_edge ***/
/********************/
Chull3D_edge::Chull3D_edge (Chull3D *hull3D)
{
 adj_faces[0]  = adj_faces[1]  = new_face = NULL;
  end_points[0] = end_points[1] = NULL;
  to_delete = 0;
  hull3D->add_edge (this);
}

/********************/
/*** Chull3D_face ***/
/********************/
Chull3D_face::Chull3D_face (Chull3D *hull3D, Chull3D_vertex *v1, Chull3D_vertex *v2, Chull3D_vertex *v3, Chull3D_face *f)
{
  Chull3D_edge *e0, *e1, *e2;

  /* create edges of the initial triangle */
  if (!f)
    {
      e0 = new Chull3D_edge (hull3D);
      e1 = new Chull3D_edge (hull3D);
      e2 = new Chull3D_edge (hull3D);
    }
  else
    {
      e0 = f->edges[2];
      e1 = f->edges[1];
      e2 = f->edges[0];
    }
  e0->end_points[0] = v1;       e0->end_points[1] = v2;
  e1->end_points[0] = v2;       e1->end_points[1] = v3;
  e2->end_points[0] = v3;       e2->end_points[1] = v1;

  /* create face for triangle */
  edges[0]    = e0;    edges[1]    = e1;    edges[2]    = e2;
  vertices[0] = v1;    vertices[1] = v2;    vertices[2] = v3;
  visible = 0;

  /* links edges to face */
  e0->adj_faces[0] = e1->adj_faces[0] = e2->adj_faces[0] = this;

  hull3D->add_face (this);
}

/*
 * makes a new face and two new edges between the edge and the point
 * that are passed to it.
 */
Chull3D_face::Chull3D_face (Chull3D *hull3D, Chull3D_edge *e, Chull3D_vertex *v)
{
  Chull3D_edge *new_edges[2];
  int i,j;

  /* make two new edges (if don't already exist)*/
  for (i=0; i<2; ++i)
    /* if the edge exists, copy it into new_edges */
    if (!(new_edges[i] = e->end_points[i]->duplicate))
      {
        /* otherwise (duplicate is NULL) */
        new_edges[i] = new Chull3D_edge (hull3D);
        new_edges[i]->end_points[0] = e->end_points[i];
        new_edges[i]->end_points[1] = v;
        e->end_points[i]->duplicate = new_edges[i];
      }

  /* make the new face */
  edges[0] = e;
  edges[1] = new_edges[0];
  edges[2] = new_edges[1];
  visible = 0;
  make_ccw (e, v);

  /* set the adjacent face pointers */
  for (i=0; i<2; ++i)
    for (j=0; j<2; ++j)
      /* only the NULL link should be set to this face */
      if (!new_edges[i]->adj_faces[j])
      {
        new_edges[i]->adj_faces[j] = this;
        break;
      }

  hull3D->add_face (this);
}

/*
 * make_ccw puts the vertices in the face structure in
 * counterclockwise order.  We want to store the vertices
 * in the same order as in the visible face.  The third
 * vertex is always p.
 *
 * Although no specific ordering of the edges of a face are
 * used by the code, the following condition is maintained
 * for each face f:
 * one of the two endpoints of f->edge[i] matches f->vertex[i].
 * But note that this does not imply that f->edge[i] is between
 * f->vertex[i] and f->vertex[(i+1)%3].  (Thanks to Bob Williamson.)
 */
void
Chull3D_face::make_ccw (Chull3D_edge *e, Chull3D_vertex *v)
{
  Chull3D_face *fv; /* the visible face adjacent to e */
  int i;            /* index of e->end_points[0] in fv */
  Chull3D_edge *s;  /* temporary, for swapping */

  if (e->adj_faces[0]->visible) fv = e->adj_faces[0];
  else                          fv = e->adj_faces[1];

  /* set vertices[0] & vertices[1] to have the same orientation as do
     the corresponding  vertices of fv */
  for (i=0; fv->vertices[i] != e->end_points[0]; ++i) {}

  /* orient this the same as fv */
  if (fv->vertices[(i+1)%3] != e->end_points[1])
    {
      vertices[0] = e->end_points[1];
      vertices[1] = e->end_points[0];
    }
  else
    {
      vertices[0] = e->end_points[0];
      vertices[1] = e->end_points[1];
      SWAP (s, edges[1], edges[2]);
    }

  /* this swap is tricky. e is edges[0]. edges[1] is based on end_points[0],
     edges[2] on end_points[1]. So if e is oriented "forwards", we need to
     move  edges[1] to follow [0], because it precedes */
  vertices[2] = v;
}
