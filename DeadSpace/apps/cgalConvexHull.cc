/*
 * computeCH.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: sidlausk
 *
 *  Computes 2D convex hull (for each node) using CGAL library (http://www.cgal.org).
 */

#include <unistd.h>
#include <stdint.h>

#include <cstdio>
#include <iomanip>
#include <vector>

#include "Color.h"
#include "VisUtils.h"
#include "ASCIDumpParser.h"
#include "NodeVisualizer.h"
#include "Utils.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

//#include <CGAL/Cartesian.h>
//typedef CGAL::Cartesian<double> K;

#include <CGAL/IO/Geomview_stream.h>
#include <CGAL/IO/Polyhedron_geomview_ostream.h>
#include <CGAL/IO/Polyhedron_geomview_ostream.h>
//#include <CGAL/IO/Triangulation_geomview_ostream_2.h>
//#include <CGAL/IO/Triangulation_geomview_ostream_3.h>

#if NUM_DIMS == 2
#include <CGAL/convex_hull_2.h>
#include <CGAL/Polygon_2.h>

typedef K::Point_2 CGALPoint;
typedef CGAL::Polygon_2<K> CGALPoly;

#else

#include <CGAL/algorithm.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/convex_hull_3.h>

typedef CGAL::Polyhedron_3<K>                      CGALPoly;
typedef CGALPoly::Facet_iterator                   Facet_iterator;
typedef CGALPoly::Halfedge_around_facet_circulator Halfedge_facet_circulator;
typedef CGALPoly::Halfedge_const_handle            HE_handle;
typedef K::Segment_3                               CGALSegment;

// define point creator
typedef K::Point_3                                CGALPoint;

//a functor computing the plane containing a triangular facet
struct Plane_from_facet {
  CGALPoly::Plane_3 operator()(CGALPoly::Facet& f) {
    CGALPoly::Halfedge_handle h = f.halfedge();
      return CGALPoly::Plane_3( h->vertex()->point(),
                                h->next()->vertex()->point(),
                                h->opposite()->vertex()->point());
  }
};

/* For Point in Polyhedron Test */
//#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/Side_of_triangle_mesh.h>

//typedef CGAL::Simple_cartesian<double> K;
typedef CGAL::AABB_face_graph_triangle_primitive<CGALPoly> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;
typedef CGAL::Side_of_triangle_mesh<CGALPoly, K> Point_inside;

#endif

using namespace std;

static uint32_t ERROR_COUNT = 0;

void printUsage(char *prog) {
  printf("\n%s - computes CH for each node in RStar ASCI dump file using CGAL\n\n", prog);

  printf("USAGE: ./%s -i ASCI_treedump [-h | -v | -l llevel | -n 'list of nodes ids']\n", prog);
  printf(" -h: this help message\n");
  printf(" -v: verbose\n");
  printf(" -i: input file with (.RSF.ASC) extension\n");
  printf(" -l: lowest level to compute dead-space (default: 0, i.e., leaf-level\n");
  printf(" -n: string of node ids that will be visualized. Note: root node id =0,\n"
         "     non-leaf node id = [1..m], leaf node id = [1..n]. As such, there \n"
         "     might be two nodes with the same id.\n");
  printf("Example: ");
  printf("%s -v -i tests/test_data.RSF.ASC -n \"0 1 2\" -o\n\n", prog);
}

/*
 * Computes pixel-based (absolute and approximate) volume of node's convex hull.
 *
 * The accuracy depends on the macro PIXELS_PER_DIMENSIONS (usually at least
 * 100 voxels for longest dimension are dedicated so that accuracy is high).
 *
 * The CH volume is stored in node.
 */
void computePixelCHVol3D( NodeInfo &node, const CGALPoly &poly ) {
  if ( node.mbr.volume() == 0.0 ) {
    assert(false);
    return;
  }

#if NUM_DIMS == 3

  /* COMPUTE PIXEL-BASED VOLUME */
  // Construct AABB tree with a KdTree
  Tree tree(faces(poly).first, faces(poly).second, poly);
  tree.accelerate_distance_queries();
  // Initialize the point-in-polyhedron tester
  Point_inside inside_tester(tree);

  const Point diff = node.mbr.lo.distanceTo(node.mbr.hi);
  Point cell_size;
  vector<int32_t> grid_rez(3);
  double max_side = diff[0];
  for (uint32_t d = 1; d < 3; ++d) {
    max_side = std::max(max_side, diff[d]);
  }

  uint64_t num_total_pixels = 1;
  for (uint32_t d = 0; d < 3; ++d) {
    grid_rez[d] = std::ceil( (diff[d] * PIXELS_PER_DIMENSIONS) / max_side );
    num_total_pixels *= grid_rez[d];
    cell_size[d] = diff[d] / grid_rez[d];
  }
//  printf( "GRID REZO: %u x %u x %u\n", grid_rez[0], grid_rez[1], grid_rez[2] );
//  printf( "CELL SIZE: %f x %f x %f\n", cell_size[0], cell_size[1], cell_size[2] );
//  VisUtils::plotGrid(node.mbr, grid_rez, "tests/grid", true, WHITE_COLOR);

  assert( num_total_pixels > 0 );

  assert( node.ch_pixels.empty() );
//  VisUtils::reset("tests/", "voxelized_ch_", true );
  for (int32_t x = 0; x < grid_rez[0]; ++x) {
    double x1 = node.mbr.lo[0] + x * cell_size[0];
    double x2 = node.mbr.lo[0] + (x+1) * cell_size[0];

    for (int32_t y = 0; y < grid_rez[1]; ++y) {
      double y1 = node.mbr.lo[1] + y * cell_size[1];
      double y2 = node.mbr.lo[1] + (y+1) * cell_size[1];
      for (int32_t z = 0; z < grid_rez[2]; ++z) {
        double z1 = node.mbr.lo[2] + z * cell_size[2];
        double z2 = node.mbr.lo[2] + (z+1) * cell_size[2];
        Box voxel(
            Point(std::initializer_list<double>{x1, y1, z1}),
            Point(std::initializer_list<double>{x2, y2, z2})
        );
        Point center = voxel.getCenter();
        CGALPoint cgal_point( center[0], center[1], center[2] );

        if ( inside_tester(cgal_point) == CGAL::ON_BOUNDED_SIDE ) {
          node.ch_pixels.push_back( voxel );
//          VisUtils::addAndHold( voxel, false, BLACK_COLOR );
        }
      }
    }
  }

  node.convex_hull_area = node.mbr.volume() * node.ch_pixels.size() / num_total_pixels;
  assert( node.convex_hull_area <= node.mbr.volume() );

#endif
}

void cgalConvexHull( NodeInfo &node, bool verbose = true ) {

#if NUM_DIMS == 2
  vector<CGALPoint> cgal_points, cgal_rslt;

  for (uint32_t i = 0; i < node.entries.size(); ++i) {
    for (uint32_t c = 0; c < NUM_CORNERS; ++c) {
      Point corner = node.entries[i].getCorner(c);
      cgal_points.push_back( CGALPoint(corner[0], corner[1]) );
    }
  }
  CGAL::convex_hull_2( cgal_points.begin(), cgal_points.end(), std::back_inserter(cgal_rslt) );

//  printf( "Node#%u |CH| = %lu\n", node.id, cgal_rslt.size() );

  assert( node.ch_vertices.empty() );
  CGALPoly poly;
  for (uint32_t i = 0; i < cgal_rslt.size(); ++i) {
    Point p;
    p[0] = cgal_rslt[i][0];
    p[1] = cgal_rslt[i][1];
    node.ch_vertices.push_back( p );
    poly.push_back( cgal_rslt[i] );
  }

  assert( poly.is_simple() );
  assert( poly.is_convex() );
  assert( poly.orientation() == CGAL::CLOCKWISE );
  node.convex_hull_area = poly.area();
  node.ch_size = cgal_rslt.size();

#else

  vector<CGALPoint> cgal_points;

  for (uint32_t i = 0; i < node.entries.size(); ++i) {
    for (uint32_t c = 0; c < NUM_CORNERS; ++c) {
      Point corner = node.entries[i].getCorner(c);
      cgal_points.push_back( CGALPoint(corner[0], corner[1], corner[2]) );
    }
  }
  // define polyhedron to hold convex hull
  CGALPoly poly;

  // compute convex hull of non-collinear points
  CGAL::convex_hull_3( cgal_points.begin(), cgal_points.end(), poly );
  for (auto pit =  poly.points_begin(); pit != poly.points_end(); ++pit) {
    Point p;
    p[0] = (*pit)[0];
    p[1] = (*pit)[1];
    p[2] = (*pit)[2];

    assert( p[0] >= node.mbr.lo[0] && p[0] <= node.mbr.hi[0] );
    assert( p[1] >= node.mbr.lo[1] && p[1] <= node.mbr.hi[1] );
    assert( p[2] >= node.mbr.lo[2] && p[2] <= node.mbr.hi[2] );
    node.ch_vertices.push_back(p);
  }

  /*
   * TRYING GEOMVIEW (v1.9.5)
   */
  CGAL::Geomview_stream gv( CGAL::Bbox_3(node.mbr.lo[0], node.mbr.lo[1], node.mbr.lo[2],
      node.mbr.hi[0], node.mbr.hi[1], node.mbr.hi[2]) );
  gv.set_bg_color( CGAL::WHITE );
  gv.clear(); // remove the pickplane.
//  gv.set_line_width(4);
//  gv.set_trace(true);
  gv.set_wired( false );

  // Visualize node's MBB
  gv << CGAL::Bbox_3( node.mbr.lo[0], node.mbr.lo[1], node.mbr.lo[2],
                      node.mbr.hi[0], node.mbr.hi[1], node.mbr.hi[2] );

  // Visualize node's data (each spatial object):
  gv << CGAL::GRAY;
  for (auto o : node.entries)
    gv << CGAL::Bbox_3(o.lo[0], o.lo[1], o.lo[2], o.hi[0], o.hi[1], o.hi[2]);

  // Visualize vertices of CH:
  gv << CGAL::RED;
  for (auto pit =  poly.points_begin(); pit != poly.points_end(); ++pit)
    gv << *pit;

  for (auto fit = poly.facets_begin(); fit != poly.facets_end(); ++fit) {

    CGALPoly::Halfedge_handle h = fit->halfedge();
    CGALPoint v1 = h->vertex()->point();
    CGALPoint v2 = h->next()->vertex()->point();
    CGALPoint v3 = h->opposite()->vertex()->point();

    Point p1, p2, p3;
    p1[0] = v1[0];
    p1[1] = v1[1];
    p1[2] = v1[2];
    p2[0] = v2[0];
    p2[1] = v2[1];
    p2[2] = v2[2];
    p3[0] = v3[0];
    p3[1] = v3[1];
    p3[2] = v3[2];

    std::vector<Point> triangle;
    triangle.push_back(p1);
    triangle.push_back(p2);
    triangle.push_back(p3);
    node.ch_planes.push_back( triangle );

    // Visualize each triangle plane of CH:
//    gv << CGAL::GREEN << K::Triangle_3 (h->vertex()->point(),
//                             h->next()->vertex()->point(),
//                             h->opposite()->vertex()->point() );
  }

  // Visualize the entire CH:
//  gv << CGAL::DEEPBLUE << poly;
  gv.look_recenter();

  // assign a plane equation to each polyhedron facet using functor Plane_from_facet
//  std::transform( poly.facets_begin(), poly.facets_end(), poly.planes_begin(),Plane_from_facet());

  computePixelCHVol3D( node, poly );

  node.ch_size = poly.size_of_vertices();

//  printf( "Node#%u |CH-vertices| = %lu\n", node.id, poly.size_of_vertices() );
//  printf( "Node#%u |CH-halfedges| = %lu\n", node.id, poly.size_of_halfedges() );
//  printf( "Node#%u |CH-facets| = %lu\n", node.id, poly.size_of_facets() );

#endif
}

void outputAllNodeDetails( const NodeInfo &node, string outfile_suffix ) {

  printf( "Node#%u COVERAGE:\n", node.id );
  if ( node.exact_coverage_abs > 0 )
    printf(" `-> EXACT: %10.0f\n", node.exact_coverage_abs);
  if ( node.pixel_coverage_abs > 0 )
    printf(" `-> PIXEL: %10.0f\n", node.pixel_coverage_abs);

  double cov = 0.0;
  for (auto e: node.entries) {
    cov += e.volume();
  }
  printf( " `-> OVERL: %10.0f\t\t\t<--- exact but over counts overlap!\n", cov );
  printf( " `->   MBB: %10.0f\n", node.mbr.volume() );
  printf( " `->    CH: %10.0f\t\t\t[vertices=%lu, triangles=%lu]\n", node.convex_hull_area,
      node.ch_vertices.size(), node.ch_planes.size() );
  printf("---\n");
  NodeVisualizer::vis( node, outfile_suffix );
}

void writeOutput( std::ofstream &ofs, const NodeInfo &node ) {
  //     "node level #entries"
  ofs << node.id << " " << node.level << " " << node.size << " "
      // exact-coverage
      << node.exact_coverage_abs << " "
      // MBB and CH coverages:
      << node.mbr.volume() << " "
//      << (node.mbr.volume() - node.exact_coverage_abs) / node.mbr.volume() << " "
      << node.convex_hull_area << " "
//      << (node.convex_hull_area - node.exact_coverage_abs) / node.convex_hull_area << " "
//        << (node.convex_hull_area - node.exact_coverage_abs) / node.convex_hull_area << " "
      // "|CH|" --- #vertices in CH
      << node.ch_size << " ";

  ofs << endl;
}

int main(int argc, char **argv) {

  string inputfile = "";
  uint32_t llevel = 0;
  bool vis = false;
  vector<uint32_t> nodeids;
  bool verbose = false;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "i:l:n:m:ovsh")) != -1) {
    switch (c) {
    case 'h':
      printUsage( argv[0] );
      return 1;
    case 'i':
      inputfile = string(optarg);
      break;
    case 'l':
      llevel = atoi(optarg);
      break;
    case 'n': {
      istringstream sin( (string(optarg)) );
      uint32_t id = 0;
      while (sin >> id) {
        nodeids.push_back( id );
      }
      vis = true;
      assert( !nodeids.empty() );
      break;
    }
    case 'v':
      verbose = true;
      break;
    default:
      fprintf( stderr, "Unknown option `-%c'.\n", optopt);
      printUsage( argv[0] );
      return 1;
    }
  }

  if (argc == 1 || optind != argc) {
    printf("argc = %d, optind = %d\n", argc, optind);
    printf("next remaining opt: %s \n", argv[optind]);
    printUsage( argv[0] );
    return 1;
  }

  ASCIDumpParser parser( inputfile, llevel );
  assert( parser.num_dims() == NUM_DIMS );

  std::string outfilename = parser.index_name() + "_" + parser.data_name() + "_cgalCH.csv";
  std::ofstream ofs( outfilename );
  // Output file header:
  ofs << "node level #entries exact-coverage MBB CH |CH|" << endl;
  ofs << std::fixed << std::setprecision(18);

  NodeInfo node;
  while ( parser.getNextNode(node) ) {

    if ( node.level < llevel ) continue;
    if ( vis && std::find(nodeids.begin(), nodeids.end(), node.id) == nodeids.end() )
      continue;

    node.mbr = Box::computeMBR( node.entries );

    cgalConvexHull( node );

    Utils::computeDeadspace( node, true );

    if ( vis && std::find(nodeids.begin(), nodeids.end(), node.id) != nodeids.end() ) {
      // Output all the details about this node including its visualization:
      string suffix = parser.index_name() + "_";
      outputAllNodeDetails( node, suffix );
    }

#if NUM_DIMS == 2
    node.exact_coverage_abs = Box::totalArea2d( node.entries );
#else
    node.exact_coverage_abs = node.pixel_coverage_abs;
#endif

    writeOutput( ofs, node );

    if ( verbose && parser.nodes_parsed() % 1000) {
      printf( "Nodes processed: %uk\r", parser.nodes_parsed() / 1000 );
      fflush(stdout);
    }
  } // GET NEXT NODE

  printf("\n");
  if (verbose) {
    printf( "DONE!\n" );
    printf( "'-> #lines: %ld\n", parser.line_counter() );
    printf( "'-> #visualization files saved: %u\n", VisUtils::files_saved() );
    printf( "'-> #nodes parsed: %u\n", parser.nodes_parsed() );
    printf( "'-> Stats about each node written to: %s\n", outfilename.c_str() );
  }

  if (  ERROR_COUNT != 0 )
    printf("ERRORS: %u\n", ERROR_COUNT );

  ofs.close();

  return EXIT_SUCCESS;
}
