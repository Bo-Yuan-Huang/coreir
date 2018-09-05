#include <fstream>
#include "coreir.h"
#include "coreir/passes/analysis/vmodule.h"
#include "coreir/passes/analysis/verilog.h"
#include "coreir/tools/cxxopts.h"

namespace CoreIR {

namespace {

void WriteModuleToStream(const Passes::VerilogNamespace::VModule* module,
                         std::ostream& os) {
  if (module->isExternal) {
    // TODO(rsetaluri): Use "defer" like semantics to avoid duplicate calls to
    // toString().
    os << "/* External Modules" << std::endl;
    os << module->toString() << std::endl;
    os << "*/" << std::endl;
    return;
  }
  os << module->toString() << std::endl;
}

}  // namespace

void Passes::Verilog::initialize(int argc, char** argv) {
  cxxopts::Options options("verilog", "translates coreir graph to verilog and optionally inlines primitives");
  options.add_options()
    ("i,inline","Inline verilog modules if possible")
  ;
  options.parse(argc,argv);
  if (options.count("i")) {
    this->vmods._inline = true;
  }
}

std::string Passes::Verilog::ID = "verilog";
bool Passes::Verilog::runOnInstanceGraphNode(InstanceGraphNode& node) {
  
  //Create a new Vmodule for this node
  Module* m = node.getModule();
  
  vmods.addModule(m);
  return false;
}

void Passes::Verilog::writeToStream(std::ostream& os) {
  for (auto module : vmods.vmods) {
    if (vmods._inline && module->inlineable) {
      continue;
    }
    WriteModuleToStream(module, os);
  }
}

void Passes::Verilog::writeToFiles(const std::string& dir) {
  for (auto module : vmods.vmods) {
    if (vmods._inline && module->inlineable) {
      continue;
    }
    const std::string filename = dir + "/" + module->modname + ".v";
    std::ofstream fout(filename);
    ASSERT(fout.is_open(), "Cannot open file: " + filename);
    WriteModuleToStream(module, fout);
    fout.close();
  }
}

Passes::Verilog::~Verilog() {
  //set<VModule*> toDelete;
  //for (auto m : modMap) {
  //  toDelete.insert(m.second);
  //}
  //for (auto m : toDelete) {
  //  delete m;
  //}
}

}
