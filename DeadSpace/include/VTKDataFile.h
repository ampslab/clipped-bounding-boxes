#ifndef VTK_FILE_H
#define VTK_FILE_H

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include "SpatialObjects.h"

using namespace std;

typedef struct Vertex3D {
  double coord[3];

  Vertex3D(){
    coord[0] = 0;
    coord[1] = 0;
    coord[2] = 0;
  }
  Vertex3D(const Point &p, bool is3D) {
    if ( is3D ) {
      coord[0] = p[0];
      coord[1] = p[1];
      coord[2] = p[2];
    } else {
      coord[0] = p[0];
      coord[1] = p[1];
      coord[2] = 0;
    }
  }

  inline double& operator[](const size_t idx) {
    assert(idx >= 0 && idx < 3);
    return coord[idx];
  };

  inline const double& operator[](const size_t idx) const {
    assert(idx >= 0 && idx < 3);
    return coord[idx];
  }

  void print() {
    printf( "[" );
    for (uint32_t i = 0; i < 3; ++i)
      printf( "%f ", coord[i] );
    printf( "]\n" );
  }

} Vertex3D;

typedef struct Box3D {
  Vertex3D low;
  Vertex3D high;

  Box3D(const Box &mbr, bool is3D):
    low(mbr.lo, is3D), high(mbr.hi, is3D) { }
} Box3D;

typedef struct Triangle3D {
  Vertex3D v0;
  Vertex3D v1;
  Vertex3D v2;

  Triangle3D(const vector<Point> &v, bool is3D):
    v0(v[0], is3D), v1(v[1], is3D), v2(v[2], is3D) {
    assert( v.size() == 3 );
  }
} Triangle3D;

class vtkDataFile {

private:
  string _filename;
  string _mode;
  bool _saved;

  vector<Box> _points;
  std::ofstream _file;

  int _pointCount;

  int _polyCount;
  int _lineCount;
  int _vertexCount;

  int _polySize;
  int _lineSize;
  int _vertexSize;

  /*
   * More info about these here: http://www.cacr.caltech.edu/~slombey/asci/vtk/vtk_formats.simple.html
   */
  stringstream _pointSection;
  stringstream _polySection;
  stringstream _verticesSection;
  stringstream _linesSection;
  stringstream _dataSection;
  string _colorSection;

  void writeHeader() {
    _file << "# vtk DataFile Version 2.0" << endl;
    _file << "Darius VTK file writer" << endl;
    if (_mode == "ascii")
      _file << "ASCII" << endl;
    else
      _file << "BINARY" << endl;
    _file << "DATASET POLYDATA" << endl;
  }

  void writeFile() {
    _file << "POINTS " << _pointCount << " float" << endl;
    _file << _pointSection.str();

    if (_polyCount > 0) {
      _file << "POLYGONS " << _polyCount << " " << _polySize << endl;
      _file << _polySection.str();
    }

    if (_lineCount > 0) {
      _file << "LINES " << _lineCount << " " << _lineSize << endl;
      _file << _linesSection.str();
    }

    if (_vertexCount > 0) {
      _file << "VERTICES " << _vertexCount << " " << _vertexSize << endl;
      _file << _vertexCount << _verticesSection.str() << endl;
    }

    if (!_colorSection.empty()) {
      _file << "POINT_DATA " << _pointCount << endl;
      _file << "SCALARS my_scalars float" << endl;
      _file << "LOOKUP_TABLE custom_table" << endl;
      _file << _dataSection.str() << endl;
      _file << "LOOKUP_TABLE custom_table 2" << endl;
      _file << "1.0 1.0 1.0 1.0" << endl;
      _file << _colorSection << endl;
    }

    _file.flush();
  }

public:
  vtkDataFile(string fileName, string fileMode) {
    _saved = false;
    _pointCount = 0;
    _polyCount = 0;
    _lineCount = 0;
    _vertexCount = 0;
    _lineSize = 0;
    _polySize = 0;
    _lineSize = 0;
    _vertexSize = 1;
    _filename = fileName;
    _mode = fileMode;
    _file.open(_filename.c_str(), std::ios::out | std::ios::trunc);
    _verticesSection.clear();
    _linesSection.clear();
    _polySection.clear();
    _pointSection.clear();
    _colorSection.clear();
    _dataSection.clear();
  }

  ~vtkDataFile() {
    if (!_saved)
      save();
    _file.close();
  }

  void save() {
    writeHeader();
    writeFile();
    _saved = true;
  }

  void DrawVertex(const Vertex3D& v, const Color& c) {
    _pointSection << v[0] << " " << v[1] << " " << v[2] << endl;
    _verticesSection << " " << _pointCount;

    if ( !(c.rgb[0] == 255 && c.rgb[1] == 255 && c.rgb[2] == 255) ) {
      _colorSection = std::to_string(c.rgb[0] / 255) + " "
          + std::to_string(c.rgb[1] / 255) + " "
          + std::to_string(c.rgb[2] / 255) + " 1.0";
      _dataSection << "1.0" << endl;
    } else {
      _dataSection << "0.0" << endl;
    }

    _pointCount++;
    _vertexCount++;
    _vertexSize++;
  }

  void DrawLine(const Vertex3D& v1, const Vertex3D& v2) {
    _pointSection << v1[0] << " " << v1[1] << " " << v1[2] << endl;
    _pointSection << v2[0] << " " << v2[1] << " " << v2[2] << endl;
    _linesSection << "2 " << _pointCount << " " << _pointCount + 1 << endl;
    _pointCount += 2;
    _lineCount++;
    _lineSize += 3;
  }

  void DrawLine(const Vertex3D& v1, const Vertex3D& v2, const Color& c) {
    _pointSection << v1[0] << " " << v1[1] << " " << v1[2] << endl;
    _pointSection << v2[0] << " " << v2[1] << " " << v2[2] << endl;
    _linesSection << "2 " << _pointCount << " " << _pointCount + 1 << endl;
    if (!(c.rgb[0] == 255 && c.rgb[1] == 255 && c.rgb[2] == 255)) {
      _colorSection = std::to_string(c.rgb[0] / 255) + " "
          + std::to_string(c.rgb[1] / 255) + " "
          + std::to_string(c.rgb[2] / 255) + " 1.0";
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
    } else {
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
    }
    _pointCount += 2;
    _lineCount++;
    _lineSize += 3;
  }

  void DrawSquare(const Vertex3D& v1, const Vertex3D& v2, const Vertex3D& v3,
      const Vertex3D& v4, const Color& c) {
    _pointSection << v1[0] << " " << v1[1] << " " << v1[2] << endl;
    _pointSection << v2[0] << " " << v2[1] << " " << v2[2] << endl;
    _pointSection << v3[0] << " " << v3[1] << " " << v3[2] << endl;
    _pointSection << v4[0] << " " << v4[1] << " " << v4[2] << endl;
    _polySection << "4 " << _pointCount << " " << _pointCount + 1 << " "
        << _pointCount + 2 << " " << _pointCount + 3 << endl;
    _pointCount += 4;
    _polyCount++;
    _polySize += 5;
  }

  void DrawBoxTransparent(const Box& b, const Color& c) {
    const Box3D box(b, sizeof(Point) == 24);
    Vertex3D v0, v1;

    v0 = box.low;
    v1 = v0;
    v1[1] = box.high[1];
    DrawLine(v0, v1, c);
    v1 = v0;
    v1[2] = box.high[2];
    DrawLine(v0, v1, c);
    v1 = v0;
    v1[0] = box.high[0];
    DrawLine(v0, v1, c);

    v0 = box.high;
    v1 = v0;
    v1[1] = box.low[1];
    DrawLine(v0, v1, c);
    v1 = v0;
    v1[2] = box.low[2];
    DrawLine(v0, v1, c);
    v1 = v0;
    v1[0] = box.low[0];
    DrawLine(v0, v1, c);

    v0 = box.low;
    v0[1] = box.high[1];
    v1 = box.high;
    v1[0] = box.low[0];
    DrawLine(v0, v1, c);
    v1 = box.high;
    v1[2] = box.low[2];
    DrawLine(v0, v1, c);

    v0 = box.low;
    v0[2] = box.high[2];
    v1 = box.high;
    v1[0] = box.low[0];
    DrawLine(v0, v1, c);
    v1 = box.high;
    v1[1] = box.low[1];
    DrawLine(v0, v1, c);

    v0 = box.low;
    v0[0] = box.high[0];
    v1 = box.high;
    v1[2] = box.low[2];
    DrawLine(v0, v1, c);
    v1 = box.high;
    v1[1] = box.low[1];
    DrawLine(v0, v1, c);
  }
  void DrawBoxTransparent(const Box& b) {
    const Box3D box(b, sizeof(Point) == 24);
    Vertex3D v0, v1;

    v0 = box.low;
    v1 = v0;
    v1[1] = box.low[1];
    DrawLine(v0, v1);
    v1 = v0;
    v1[2] = box.high[2];
    DrawLine(v0, v1);
    v1 = v0;
    v1[0] = box.high[0];
    DrawLine(v0, v1);

    v0 = box.high;
    v1 = v0;
    v1[1] = box.low[1];
    DrawLine(v0, v1);
    v1 = v0;
    v1[2] = box.low[2];
    DrawLine(v0, v1);
    v1 = v0;
    v1[0] = box.low[0];
    DrawLine(v0, v1);

    v0 = box.low;
    v0[1] = box.high[1];
    v1 = box.high;
    v1[0] = box.low[0];
    DrawLine(v0, v1);
    v1 = box.high;
    v1[2] = box.low[2];
    DrawLine(v0, v1);

    v0 = box.low;
    v0[2] = box.high[2];
    v1 = box.high;
    v1[0] = box.low[0];
    DrawLine(v0, v1);
    v1 = box.high;
    v1[1] = box.low[1];
    DrawLine(v0, v1);

    v0 = box.low;
    v0[0] = box.high[0];
    v1 = box.high;
    v1[2] = box.low[2];
    DrawLine(v0, v1);
    v1 = box.high;
    v1[1] = box.low[1];
    DrawLine(v0, v1);
  }

  void DrawBoxesTransparent(const vector<Box> &b, const Color& c) {
    for (uint32_t i = 0; i < b.size(); ++i) {
      DrawBoxTransparent(b[i], c);
    }
  }

  void DrawBoxesTransparent(const vector<Box> &b) {
    for (uint32_t i = 0; i < b.size(); ++i) {
      DrawBoxTransparent(b[i]);
    }
  }

  void DrawBox(const Box& b, const Color& c) {
    const Box3D box(b, sizeof(Point) == 24);
    Vertex3D v0, v1, v2, v3, v4, v5, v6, v7;

    v0 = box.low;
    v1 = v0;
    v1[0] = box.high[0];
    v2 = v1;
    v2[1] = box.high[1];
    v3 = v2;
    v3[0] = box.low[0];

    v4 = v0;
    v4[2] = box.high[2];
    v5 = v1;
    v5[2] = box.high[2];
    v6 = v2;
    v6[2] = box.high[2];
    v7 = v3;
    v7[2] = box.high[2];

    _pointSection << v0[0] << " " << v0[1] << " " << v0[2] << endl;
    _pointSection << v1[0] << " " << v1[1] << " " << v1[2] << endl;
    _pointSection << v2[0] << " " << v2[1] << " " << v2[2] << endl;
    _pointSection << v3[0] << " " << v3[1] << " " << v3[2] << endl;
    _pointSection << v4[0] << " " << v4[1] << " " << v4[2] << endl;
    _pointSection << v5[0] << " " << v5[1] << " " << v5[2] << endl;
    _pointSection << v6[0] << " " << v6[1] << " " << v6[2] << endl;
    _pointSection << v7[0] << " " << v7[1] << " " << v7[2] << endl;

    _polySection << "4 " << _pointCount + 0 << " " << _pointCount + 1 << " "
        << _pointCount + 2 << " " << _pointCount + 3 << endl;
    _polySection << "4 " << _pointCount + 4 << " " << _pointCount + 5 << " "
        << _pointCount + 6 << " " << _pointCount + 7 << endl;
    _polySection << "4 " << _pointCount + 0 << " " << _pointCount + 1 << " "
        << _pointCount + 5 << " " << _pointCount + 4 << endl;
    _polySection << "4 " << _pointCount + 2 << " " << _pointCount + 3 << " "
        << _pointCount + 7 << " " << _pointCount + 6 << endl;
    _polySection << "4 " << _pointCount + 0 << " " << _pointCount + 4 << " "
        << _pointCount + 7 << " " << _pointCount + 3 << endl;
    _polySection << "4 " << _pointCount + 1 << " " << _pointCount + 2 << " "
        << _pointCount + 6 << " " << _pointCount + 5 << endl;

    if (!(c.rgb[0] == 255 && c.rgb[1] == 255 && c.rgb[2] == 255)) {
      _colorSection = std::to_string(c.rgb[0] / 255) + " "
          + std::to_string(c.rgb[1] / 255) + " "
          + std::to_string(c.rgb[2] / 255) + " 1.0";
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
    } else {
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
    }

    _pointCount += 8;
    _polyCount += 6;
    _polySize += 30;
  }

  // NEW
  void DrawPolygonTransparent( const vector<Point> &v, const Color& c ) {
    assert( v.size() >= 3 );

    for (uint32_t i = 0; i < v.size() - 1; ++i) {
      Vertex3D v0( v[i], sizeof(Point) == 24 );
      Vertex3D v1( v[i + 1], sizeof(Point) == 24 );
      DrawLine( v0, v1, c );
    }
    Vertex3D v0( v.back(), sizeof(Point) == 24 );
    Vertex3D v1( v.front(), sizeof(Point) == 24 );
    DrawLine( v0, v1, c );

  }
  void DrawPolygonTransparent( const vector<Point> &v ) {
    assert( v.size() >= 3 );

    for (uint32_t i = 0; i < v.size(); ++i) {
      Vertex3D v0( v[i], sizeof(Point) == 24 );
      Vertex3D v1( v[i + 1], sizeof(Point) == 24 );
      DrawLine( v0, v1 );
    }
    Vertex3D v0( v.back(), sizeof(Point) == 24 );
    Vertex3D v1( v.front(), sizeof(Point) == 24 );
    DrawLine( v0, v1 );
  }

  void DrawTriangle(const vector<Point> &v, const Color& c) {
    bool is3D = (sizeof(Point) == 24);
    Triangle3D t(v, is3D);

    _pointSection << t.v0[0] << " " << t.v0[1] << " " << t.v0[2] << endl;
    _pointSection << t.v1[0] << " " << t.v1[1] << " " << t.v1[2] << endl;
    _pointSection << t.v2[0] << " " << t.v2[1] << " " << t.v2[2] << endl;

    _polySection << "3 " << _pointCount + 0 << " " << _pointCount + 1 << " "
        << _pointCount + 2 << endl;

    if (!(c.rgb[0] == 255 && c.rgb[1] == 255 && c.rgb[2] == 255)) {
      _colorSection = std::to_string(c.rgb[0] / 255) + " "
          + std::to_string(c.rgb[1] / 255) + " "
          + std::to_string(c.rgb[2] / 255) + " 1.0";
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
      _dataSection << "1.0" << endl;
//      _dataSection << "1.0" << endl;
//      _dataSection << "1.0" << endl;
//      _dataSection << "1.0" << endl;
//      _dataSection << "1.0" << endl;
//      _dataSection << "1.0" << endl;
    } else {
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
      _dataSection << "0.0" << endl;
//      _dataSection << "0.0" << endl;
//      _dataSection << "0.0" << endl;
//      _dataSection << "0.0" << endl;
//      _dataSection << "0.0" << endl;
//      _dataSection << "0.0" << endl;
    }

    _pointCount += 3; //8;
    _polyCount += 1; //6;
    _polySize += 4;
  }

};

#endif
