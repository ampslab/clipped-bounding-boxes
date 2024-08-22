/*
 * VisUtils.cc
 *
 *  Created on: Jun 16, 2015
 *      Author: sidlausk
 */

#include "VisUtils.h"

#include "../include/Color.h"

uint32_t VisUtils::files_saved_ = 0;
uint32_t VisUtils::seq_no_ = 0;
string VisUtils::outfolder_ = "vis/";
string VisUtils::outfileprefix_ = "VisUtils_[0]";
vector<Box> VisUtils::boxes_;
vector<pair<bool, Color> > VisUtils::box_props_;
vector<Point> VisUtils::points_;
vector<Color> VisUtils::point_props_;

VisUtils::VisUtils() {

}

VisUtils::~VisUtils() {
  // TODO Auto-generated destructor stub
}

#define VIS

#ifdef VIS

void VisUtils::addAndPlot( const vector<Box>& b, bool transparent,
    const Color& c ) {
//  boxes_.insert( boxes_.end(), b.begin(), b.end() );
  for (uint32_t i = 0; i < b.size(); ++i) {
    boxes_.push_back( b[i] );
    box_props_.push_back( make_pair(transparent, c) );
  }
  assert(boxes_.size() == box_props_.size() );
  plotAll();
}

void VisUtils::addAndHold( const vector<Box>& b, bool transparent,
    const Color& c ) {
  for (uint32_t i = 0; i < b.size(); ++i) {
    boxes_.push_back( b[i] );
    box_props_.push_back( make_pair(transparent, c) );
  }
  assert(boxes_.size() == box_props_.size() );
}

void VisUtils::addAndHold( const Box& b, bool transparent, const Color& c ) {

  boxes_.push_back( b );
  box_props_.push_back( make_pair(transparent, c) );

  assert(boxes_.size() == box_props_.size() );
}

void VisUtils::addAndHold( const Point& p, const Color& c ) {
  points_.push_back( p );
  point_props_.push_back( c );
  assert(points_.size() == point_props_.size() );
}

void VisUtils::addAndPlot( const vector<Point>& p, const Color& c ) {
  for (uint32_t i = 0; i < p.size(); ++i) {
    points_.push_back( p[i] );
    point_props_.push_back( c );
  }
  assert(points_.size() == point_props_.size() );
  plotAll();
}

void VisUtils::addAndPlot( const Box &b, bool transparent, const Color& c ) {
  boxes_.push_back( b );
  box_props_.push_back( make_pair(transparent, c) );
  assert(boxes_.size() == box_props_.size() );
  plotAll();
}

void VisUtils::clearAndPlot( const Box &b, bool transparent, const Color& c ) {
  boxes_.clear();
  box_props_.clear();
  points_.clear();
  point_props_.clear();

  boxes_.push_back( b );
  box_props_.push_back( make_pair(transparent, c) );
  assert(boxes_.size() == box_props_.size() );
  plotAll();
}

void VisUtils::clearAndPlot( const vector<Box>& b, bool transparent,
    const Color& c ) {
  boxes_.clear();
  box_props_.clear();
  points_.clear();
  point_props_.clear();

  for (uint32_t i = 0; i < b.size(); ++i) {
    boxes_.push_back( b[i] );
    box_props_.push_back( make_pair(transparent, c) );
  }
  assert(boxes_.size() == box_props_.size() );
  plotAll();
}

void VisUtils::clearAndAdd( const Box &b, bool transparent, const Color& c ) {
  boxes_.clear();
  box_props_.clear();
  points_.clear();
  point_props_.clear();

  boxes_.push_back( b );
  box_props_.push_back( make_pair(transparent, c) );
  assert(boxes_.size() == box_props_.size() );
}

void VisUtils::addAndPlot( const Point &p, const Color& c ) {
  points_.push_back( p );
  point_props_.push_back( c );
  assert(points_.size() == point_props_.size() );
  plotAll();
}

void VisUtils::plotBoxes( const vector<Box>& b, string path, bool transparent,
    const Color& c) {

  if (b.empty()) return;

  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  for (uint32_t i = 0; i < b.size(); ++i) {
    if (transparent)
      vtk.DrawBoxTransparent(b[i], c);
    else
      vtk.DrawBox(b[i], c);
  }
  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: " << b.size() << " boxes saved to "
//       << outfile.c_str() << endl;
}

void VisUtils::plotPoints(const vector<Point>& p, string path, const Color& c) {
  if (p.empty())
    return;

  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  for (uint32_t i = 0; i < p.size(); ++i) {
    vtk.DrawVertex( Vertex3D(p[i], sizeof(Point) == 24), c );
  }
  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: " << p.size() << " points saved to "
//       << outfile.c_str() << endl;
}

void VisUtils::plotPoint(const Point &p, string path, const Color& c) {
  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  vtk.DrawVertex( Vertex3D(p, sizeof(Point) == 24), c );
  vtk.save();
  VisUtils::files_saved_++;
}

void VisUtils::plotBox(const Box& b, string path, bool transparent,
    const Color& c) {
  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  if (transparent)
    vtk.DrawBoxTransparent(b, c);
  else
    vtk.DrawBox(b, c);
  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: 1 box saved to " << outfile.c_str() << endl;
}

void VisUtils::plotPolygon(const vector<Point>& p, string path, bool transparent,
    const Color& c) {
  if (p.empty())
    return;
  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  if (transparent)
    vtk.DrawPolygonTransparent(p, c);
  else
//    vtk.DrawPolygon(p, c); TODO: not supported yet
    assert(false);
  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: 1 box saved to " << outfile.c_str() << endl;
}

void VisUtils::plotPolygons( const vector<vector<Point> >& p, string path, bool transparent,
    const Color& c) {
  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  for (uint32_t i = 0; i < p.size(); ++i) {
    if (transparent)
      vtk.DrawPolygonTransparent(p[i], c);
    else
      vtk.DrawTriangle(p[i], c);
  }
  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: " << b.size() << " boxes saved to "
//       << outfile.c_str() << endl;
}

void VisUtils::plotTriangle(const vector<Point>& t, string path, bool transparent,
    const Color& c) {
  if (t.empty())
    return;
  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  if (transparent)
    vtk.DrawPolygonTransparent(t, c);
  else {
    assert( t.size() == 3 );
    vtk.DrawTriangle(t, c);
  }
  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: 1 box saved to " << outfile.c_str() << endl;
}

void VisUtils::plotLine(const Line &l, string path, const Color& c) {
  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  Vertex3D v0( l.first, sizeof(Point) == 24 );
  Vertex3D v1( l.second, sizeof(Point) == 24 );
  vtk.DrawLine( v0, v1, c );

  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: 1 box saved to " << outfile.c_str() << endl;
}

void VisUtils::plotLines(const vector<Line>& l, string path, const Color& c) {
  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  for (uint32_t i = 0; i < l.size(); ++i) {
    Vertex3D v0( l[i].first, sizeof(Point) == 24 );
    Vertex3D v1( l[i].second, sizeof(Point) == 24 );
    vtk.DrawLine( v0, v1, c );
  }

  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: 1 box saved to " << outfile.c_str() << endl;
}

void VisUtils::plotGrid(const Box& universe, const vector<int32_t> &rezolution,
    string path, bool transparent, const Color& c) {

  const string outfile = path + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  Point diff = universe.lo.distanceTo( universe.hi );
  Point sizes;
  for (uint32_t d = 0; d < NUM_DIMS; ++d)
    sizes[d] = diff[d] / rezolution[d];

  // x-lines
  const double z1 = universe.lo[2];
  const double z2 = universe.hi[2];
  for (int32_t y = 0; y < rezolution[0]; ++y) {
    const double y1 = universe.lo[1] + y * sizes[1];

    for (int32_t x = 0; x <= rezolution[0]; ++x) {
      const double x1 = universe.lo[0] + x * sizes[0];
      Point p1( std::initializer_list<double> { x1, y1, z1 } );
      Point p2( std::initializer_list<double> { x1, y1, z2 } );
      Vertex3D v0( p1, sizeof(Point) == 24 );
      Vertex3D v1( p2, sizeof(Point) == 24 );
      vtk.DrawLine( v0, v1, c );
    }
  }

  // y-lines
  const double x1 = universe.lo[0];
  const double x2 = universe.hi[0];
  for (int32_t z = 0; z < rezolution[2]; ++z) {
    const double z1 = universe.lo[2] + z * sizes[1];

    for (int32_t y = 0; y <= rezolution[1]; ++y) {
      const double y1 = universe.lo[1] + y * sizes[1];
      Point p1( std::initializer_list<double> { x1, y1, z1 } );
      Point p2( std::initializer_list<double> { x2, y1, z1 } );
      Vertex3D v0( p1, sizeof(Point) == 24 );
      Vertex3D v1( p2, sizeof(Point) == 24 );
      vtk.DrawLine( v0, v1, c );
    }
  }

  // z-lines
  const double y1 = universe.lo[1];
  const double y2 = universe.hi[1];
  for (int32_t x = 0; x < rezolution[0]; ++x) {
    const double x1 = universe.lo[0] + x * sizes[0];

    for (int32_t z = 0; z <= rezolution[2]; ++z) {
      const double z1 = universe.lo[2] +  z * sizes[2];
      Point p1( std::initializer_list<double> { x1, y1, z1 } );
      Point p2( std::initializer_list<double> { x1, y2, z1 } );
      Vertex3D v0( p1, sizeof(Point) == 24 );
      Vertex3D v1( p2, sizeof(Point) == 24 );
      vtk.DrawLine( v0, v1, c );
    }
  }

  vtk.save();
  VisUtils::files_saved_++;
//  cout << "'-> VisUtils: 1 box saved to " << outfile.c_str() << endl;
}

#else
void VisUtils::addAndPlot( const vector<Box>& b, bool transparent,
    const Color& c ) {

}

void VisUtils::addAndPlot( const vector<Point>& p, const Color& c ) {

}

void VisUtils::addAndPlot( const Box &b, bool transparent, const Color& c ) {

}

void VisUtils::addAndPlot( const Point &p, const Color& c ) {

}

void VisUtils::plotBoxes( const vector<Box>& b, string name, bool transparent,
    const Color& c) {

}

void VisUtils::plotPoints(const vector<Point>& p, string name, const Color& c) {

}

void VisUtils::plotBox(const Box& b, string name, bool transparent,
    const Color& c) {

}
#endif

void VisUtils::plotAll() {
  const string outfile = outfolder_ + "/" + outfileprefix_ + "_"
      + to_string(seq_no_) + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  for (uint32_t i = 0; i < points_.size(); ++i) {
    vtk.DrawVertex( Vertex3D(points_[i], sizeof(Point) == 24), point_props_[i] );
  }

  for (uint32_t i = 0; i < boxes_.size(); ++i) {
    if ( box_props_[i].first )
      vtk.DrawBoxTransparent( boxes_[i], box_props_[i].second );
    else
      vtk.DrawBox( boxes_[i], box_props_[i].second );
  }

  vtk.save();
  ++files_saved_;
  ++seq_no_;
}

void VisUtils::plotAllin1() {
  const string outfile = outfolder_ + "/" + outfileprefix_ + ".vtk";
  vtkDataFile vtk(outfile, "ascii");

  for (uint32_t i = 0; i < points_.size(); ++i) {
    vtk.DrawVertex( Vertex3D(points_[i], sizeof(Point) == 24), point_props_[i] );
  }

  for (uint32_t i = 0; i < boxes_.size(); ++i) {
    if ( box_props_[i].first )
      vtk.DrawBoxTransparent( boxes_[i], box_props_[i].second );
    else
      vtk.DrawBox( boxes_[i], box_props_[i].second );
  }

  vtk.save();
  ++files_saved_;
  ++seq_no_;
}
