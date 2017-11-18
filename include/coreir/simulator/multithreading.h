#pragma once

#include "coreir/simulator/op_graph.h"

namespace CoreIR {

  typedef NGraph ThreadGraph;

  int numThreads(const ThreadGraph& g);

  void balancedComponentsParallel(NGraph& gr);

}
