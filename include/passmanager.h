#ifndef PASSMANAGER_HPP_
#define PASSMANAGER_HPP_

#include "coreir.h"
#include "passes.h"
#include "instancegraph.h"
#include <stack>

namespace CoreIR {

class InstanceGraph;
class PassManager {
  Context* c;
  
  //TODO Ad hoc, Find better system
  //Even better, construct this using a pass that is dependent
  //Need to add Analysys passes that can be used as dependencies
  //InstanceGraph* instanceGraph;
  //bool instanceGraphStale = true;

  //std::map<uint,unordered_map<uint,vector<Pass*>>> passOrdering;
  //Data structure for storing passes
  std::unordered_map<string,Pass*> passMap;

  //Name to isValid
  std::unordered_map<string,bool> analysisPasses;
  
  public:
    typedef vector<std::string> PassOrder;
    explicit PassManager(Context* c);
    ~PassManager();
    Context* getContext() { return c;}
    //This will memory manage pass.
    void addPass(Pass* p);
    //Returns if graph was modified
    //Will also remove all the passes that were run
    bool run(PassOrder order);

    Pass* getAnalysisPass(std::string ID) {
      assert(passMap.count(ID));
      return passMap[ID];
    }

  private:
    void pushAllDependencies(string oname,stack<string> &work);

    friend class Pass;
    bool runPass(Pass* p);
    bool runNamespacePass(Pass* p);
    bool runModulePass(Pass* p);
    bool runInstanceGraphPass(Pass* p);
};

}
#endif //PASSMANAGER_HPP_
