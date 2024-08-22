/*
 * ASCIDumpParser.cc
 *
 *  Created on: Jan 7, 2016
 *      Author: sidlausk
 */

#include "ASCIDumpParser.h"

ASCIDumpParser::ASCIDumpParser(std::string inputfile, const uint32_t llevel) :
    llevel_(llevel), ifs_(inputfile.c_str()) {

  num_dims_ = -1;
  data_name_ = "";
  index_name_ = inputfile.substr( inputfile.find_last_of("/\\") + 1 );
  index_name_ = index_name_.substr( 0, index_name_.find_first_of("_") );
  line_counter_ = 0;
  nodes_parsed_ = 0;

  std::string line, temp1, temp2;

  while ( getline(ifs_, line) ) {
    ++line_counter_;
    if (line == "-----")
      break;
    std::istringstream iss(line);
    if (!(iss >> temp1 >> temp2)) {
      printf("Error: parsing file header!\n");
      break; // error
    }
    if (temp1 == "name:") {
      data_name_ = temp2.substr( temp2.find_last_of("/\\") + 1 );
      data_name_ = data_name_.substr( 0, data_name_.find_first_of(".") );
    }

    if (temp1 == "numbOfDimensions:")
      num_dims_ = atoi(temp2.c_str());

  }

#ifndef NDEBUG
  //DebuggingCode
  printf( "ASCIDumpParser created for '%s' (num_dims=%d)\n", data_name_.c_str(),
      num_dims_ );
#endif
  assert( NUM_DIMS == num_dims_ && "*** Forgot to set NUM_DIMS? ***" );
}

ASCIDumpParser::~ASCIDumpParser() {
  ifs_.close();
}

bool ASCIDumpParser::getNextNode(NodeInfo &node) {
  node.reset();

  std::string line, temp1, temp2, temp3, parent;

  uint32_t lines_per_node = 0;
  // read node header
  while (getline(ifs_, line)) {
    ++lines_per_node;

    std::istringstream iss(line);
    if (!(iss >> temp1 >> temp2)) {
      printf("Error: ASCIDumpParser parsing node header!\n");
      break; // error
    }
    if (temp1 == "p:") {
      std::istringstream isstemp(temp2);
      if (!(isstemp >> node.id)) {
        printf("Error: ASCIDumpParser parsing nodeid in header!\n");
        break; // error
      }
      node.path_from_root = line;
    } else if (temp1 == "l:") {
      node.level = atoi(temp2.c_str());
    } else if (temp1 == "n:") {
      node.size = atoi(temp2.c_str());
    } else if (temp1 == "s:") {
      for (int d = 1; d < num_dims_; ++d) {
        if (!getline(ifs_, line))
          printf("Error: ASCIDumpParser parsing header\n");
        ++lines_per_node;
      }
      break;
    } else if (temp1 == "e:") {
      // node entries begin, so process the first here:
      double lo, hi;
      Point ll, ur;
      std::istringstream iss(line);
      if ( !(iss >> temp1 >> lo >> hi) ) {
        printf("Error: parsing node's first element!\n");
        break; // error
      }
      assert(temp1 == "e:");
      ll[0] = lo;
      ur[0] = hi;

      for (int d = 1; d < num_dims_; ++d) {
        if ( !getline(ifs_, line) )
          printf("Error: ASCIDumpParser failed parsing the first ndoe entry\n");
        ++lines_per_node;
        std::istringstream iss2(line);
        if ( d < num_dims_ - 1 ) {
          if (!(iss2 >> lo >> hi)) {
            printf("Error: ASCIDumpParser in parsing middle dimensions!\n");
            break; // error
          }
        } else { // last dimension
          if (!(iss2 >> lo >> hi >> temp1 >> temp2 >> temp3)) {
            printf("Error: ASCIDumpParser in parsing last dimension!\n");
            break; // error
          }
          assert(temp1 == "->");
        }
        ll[d] = lo;
        ur[d] = hi;
      }
      node.entries.push_back(Box(ll, ur));
      break;
    }
  }
#ifndef NDEBUG
  if ( node.level > 1 )
    printf("NODE-%s #%d of size=%d at level=%d\n", index_name_.c_str(), node.id,
        node.size, node.level);
#endif

  // Processing node entries:
  for (uint32_t e = node.entries.size(); e < node.size && getline(ifs_, line); ++e) {
    ++lines_per_node;
    double lo, hi;
    Point ll, ur;
    std::istringstream iss(line);
    if (!(iss >> temp1 >> lo >> hi)) {
      printf("Error: parsing node element #%d!\n", e);
      break; // error
    }
    assert(temp1 == "e:");
    ll[0] = lo;
    ur[0] = hi;

    for (int d = 1; d < num_dims_; ++d) {
      if (!getline(ifs_, line))
        printf("Error: ASCIDumpParser in entries\n");
      ++lines_per_node;
      std::istringstream iss2(line);
      if (d < num_dims_ - 1) {
        if (!(iss2 >> lo >> hi)) {
          printf("Error: ASCIDumpParser in parsing middle dimensions!\n");
          break; // error
        }
      } else { // last dimension
        if (!(iss2 >> lo >> hi >> temp1 >> temp2 >> temp3)) {
          printf("Error: ASCIDumpParser in parsing last dimension!\n");
          break; // error
        }
        assert(temp1 == "->");
      }
      ll[d] = lo;
      ur[d] = hi;
    }
    node.entries.push_back(Box(ll, ur));
  }
  if (lines_per_node == 0) {
#ifndef NDEBUG
    printf("EOF!\n");
#endif
    return false;
  } else {
    line_counter_ += lines_per_node;
    ++nodes_parsed_;
    node.name = data_name_ + "_node[" + std::to_string(node.id) + "]";
//#ifndef NDEBUG
//    if ( node.level > 0 ) {
//      printf( " '-> lines read: %u\n", lines_per_node );
//      printf( " '-> elements parsed: %lu\n", node.entries.size() );
//    }
//#endif

    assert( node.size == node.entries.size() );
    assert( node.id != std::numeric_limits<uint32_t>::max() );
    return true;
  }
}
