#include "convexhullsolver.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/algorithm.h>
#include <CGAL/Convex_hull_traits_3.h>
#include <CGAL/convex_hull_3.h>
#include <vector>
#include "mesh.h"

//typedef CGAL::Simple_cartesian<double> K;
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Convex_hull_traits_3<K>::Polyhedron_3 Polyhedron_3;
typedef K::Point_3 Point_3;
typedef K::Segment_3 Segment_3;
typedef Polyhedron_3::Halfedge_around_facet_const_circulator Halfedge_facet_circulator;


// Convex hull vertices returned in hullPoints
bool ConvexHullSolver::run(Mesh &mesh) {
//                           QList<Vector3>& hullPoints) {
    std::vector<Point_3> points;

    foreach (const Vertex &v, mesh.vertices) {
        points.push_back(Point_3(v.pos.x, v.pos.y, v.pos.z));
    }

    // define polyhedron to hold convex hull
    CGAL::Object hull;

    // compute convex hull
    CGAL::convex_hull_3(points.begin(), points.end(), hull);


    // determine what kind of object it is
    const Polyhedron_3 *polyHull = CGAL::object_cast<Polyhedron_3>(&hull);
    if (!polyHull) {
       std::cerr << "Error: no convex hull found" << std::endl;
       return false;
    }

   std::cout << "Hull points: " << polyHull->size_of_vertices() << std::endl;

   mesh.vertices.clear();
   mesh.triangles.clear();

   // output vertices on hull
   std::map<Point_3, int> pointIndices;
   Polyhedron_3::Vertex_const_iterator it;
   int count = 0;
   for (it = polyHull->vertices_begin(); it != polyHull->vertices_end(); ++it) {
       const Point_3 &p = it->point();
       mesh.vertices.push_back(Vertex(Vector3(p.x(), p.y(), p.z())));
       pointIndices[p] = count++;
   }


   // get triangles for mesh
   for (Polyhedron_3::Facet_const_iterator it = polyHull->facets_begin(); it != polyHull->facets_end(); ++it) {
       Halfedge_facet_circulator j = it->facet_begin();
       // Facets in polyhedral surfaces are at least triangles.
       CGAL_assertion(CGAL::circulator_size(j) == 3);

       //get the three vertices
       Triangle t;
       t.a.index = pointIndices[it->halfedge()->vertex()->point()];
       t.b.index = pointIndices[it->halfedge()->next()->vertex()->point()];
       t.c.index = pointIndices[it->halfedge()->prev()->vertex()->point()];
       mesh.triangles.push_back(t);
   }

   return true;
}
