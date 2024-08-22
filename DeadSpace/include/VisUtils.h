/*
 * VisUtils.h
 *
 *  Created on: Jun 16, 2015
 *      Author: sidlausk
 */

#ifndef SRC_VISUTILS_H_
#define SRC_VISUTILS_H_

#include <string>
#include <vector>
#include <cstdlib>

#include "Color.h"
#include "VTKDataFile.h"

using namespace std;

class VisUtils {
private:

  static string outfolder_;
  static string outfileprefix_;
  static uint32_t files_saved_;
  static uint32_t seq_no_;

  static vector<Box> boxes_;
  static vector<pair<bool, Color> > box_props_;
  static vector<Point> points_;
  static vector<Color> point_props_;

public:
  VisUtils();
  virtual ~VisUtils();

  static void reset(string folder, string new_outprefix = "",
		  bool counter_reset = true ) {
    boxes_.clear();
    box_props_.clear();
    points_.clear();
    point_props_.clear();
    seq_no_ = counter_reset ? 0 : seq_no_;
    outfileprefix_ = new_outprefix;
    outfolder_ = folder;

    const string mkdir_cmd = "mkdir -p " + folder;
    const int dir_err = system( mkdir_cmd.c_str() );
    if (-1 == dir_err) {
      printf( "Error: VisUtils can't create '%s' directory!n", folder.c_str() );
      exit(1);
    }
  }

  static void addAndPlot( const vector<Box>& b, bool transparent,
      const Color& c = Color(WHITE_COLOR) );
  static void addAndHold( const vector<Box>& b, bool transparent, const Color& c );
  static void addAndHold( const Box& b, bool transparent, const Color& c );
  static void addAndHold( const Point& p, const Color& c );
  static void addAndPlot( const vector<Point>& p,
      const Color& c = Color(WHITE_COLOR) );

  static void addAndPlot( const Box &b, bool transparent,
      const Color& c = Color(WHITE_COLOR) );
  static void clearAndPlot( const Box &b, bool transparent,
      const Color& c = Color(WHITE_COLOR) );
  static void clearAndPlot( const vector<Box> &b, bool transparent,
      const Color& c = Color(WHITE_COLOR) );
  static void clearAndAdd( const Box &b, bool transparent,
      const Color& c = Color(WHITE_COLOR) );
  static void addAndPlot( const Point &p, const Color& c = Color(WHITE_COLOR) );

  static void plotBoxes( const vector<Box>& b, string name, bool transparent,
      const Color& c = Color(WHITE_COLOR) );
  static void plotPoints(const vector<Point>& p, string name, const Color& c);
  static void plotBox(const Box& b, string path, bool transparent,
      const Color& c = Color(WHITE_COLOR));
  static void plotPolygon(const vector<Point>& p, string path, bool transparent,
      const Color& c = Color(WHITE_COLOR));
  static void plotPolygons(const vector<vector<Point> >& p, string path, bool transparent,
      const Color& c = Color(WHITE_COLOR));
  static void plotTriangle(const vector<Point>& t, string path, bool transparent,
      const Color& c);
  using Line = std::pair<Point, Point>;
  static void plotLine(const Line& l, string path, const Color& c = Color(BLACK_COLOR));
  static void plotLines(const vector<Line>& l, string path, const Color& c = Color(BLACK_COLOR));
  static void plotGrid(const Box& universe, const vector<int32_t> &rezolution,
      string path, bool transparent, const Color& c);
  static void plotPoint(const Point &p, string path,
      const Color& c = Color(WHITE_COLOR));
  static void plotAll( );
  static void plotAllin1( );

  static uint32_t files_saved() { return files_saved_; }
};

#endif /* SRC_VISUTILS_H_ */
