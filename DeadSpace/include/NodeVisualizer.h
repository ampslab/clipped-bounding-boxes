/*
 * NodeVisualizer.h
 *
 *  Created on: Jan 18, 2016
 *      Author: sidlausk
 */

#ifndef NODEVISUALIZER_H_
#define NODEVISUALIZER_H_

#include <string>

#include "VisUtils.h"
#include "ASCIDumpParser.h"

using namespace std;

class NodeVisualizer {
public:
  static string CBB_TYPE;

  NodeVisualizer();
  virtual ~NodeVisualizer();

//  static void visSkyNode(const NodeInfo &node, string prefix = "");
  static void vis( const NodeInfo &node, string prefix = "" );
};

#endif /* NODEVISUALIZER_H_ */
