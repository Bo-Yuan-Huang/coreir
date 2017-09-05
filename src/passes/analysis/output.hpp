#ifndef SIM_OUTPUT_HPP_
#define SIM_OUTPUT_HPP_

#include "op_graph.hpp"

#include <deque>

namespace CoreIR {

  int compileCode(const std::string& code, const std::string& outFile);

  void writeFiles(const std::deque<vdisc>& topoOrder,
		  NGraph& g,
		  Module* mod,
		  const std::string& codeFile,
		  const std::string& hFile);

  int compileCodeAndRun(const std::deque<vdisc>& topoOrder,
			NGraph& g,
			Module* mod,
			const std::string& outFile,
			const std::string& harnessFile);

  int compileCodeAndRun(const std::string& code,
			const std::string& outFile,
			const std::string& harnessFile);
  
}

#endif
