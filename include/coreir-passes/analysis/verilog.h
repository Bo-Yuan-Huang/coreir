#ifndef COREIR_VERILOG_HPP_
#define COREIR_VERILOG_HPP_

#include "coreir.h"
#include <ostream>
#include "vmodule.hpp"

namespace CoreIR {
namespace Passes {

class Verilog : public InstanceGraphPass {
  std::unordered_map<Instantiable*,VModule*> modMap;
  std::unordered_set<Instantiable*> external;
  public :
    static std::string ID;
    Verilog() : InstanceGraphPass(ID,"Creates Verilog representation of IR",true) {}
    bool runOnInstanceGraphNode(InstanceGraphNode& node) override;
    void setAnalysisInfo() override {
      addDependency("verifyfullyconnected");
      addDependency("verifyflattenedtypes");
    }
    
    void writeToStream(std::ostream& os);
};

}
}
#endif
