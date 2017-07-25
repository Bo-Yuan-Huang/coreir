#include "coreir-pass/instancegraph.h"

using namespace CoreIR;

void InstanceGraph::clear() {
  nodeMap.clear();
  for (auto ign : sortedNodes) delete ign;
  sortedNodes.clear();
}

void InstanceGraph::sortVisit(InstanceGraphNode* node) {
  if (node->mark==2) return;
  ASSERT(node->mark!=1,"SOMEHOW not a DAG");
  node->mark = 1;
  for (auto nextnode : node->ignList) {
    cout << node->getInstantiable()->getName() << "->" << nextnode->getInstantiable()->getName() << endl;
    sortVisit(nextnode);
  }
  node->mark = 2;
  sortedNodes.push_front(node);
}

void InstanceGraph::construct(Namespace* ns) {
  //Contains all external nodes referenced
  //unordered_map<Instantiable*,InstanceGraphNode*> externalNodeMap;
  
  //Create all Nodes first
  for (auto imap : ns->getModules()) {
    nodeMap[imap.second] = new InstanceGraphNode(imap.second,false);
  }
  for (auto imap : ns->getGenerators()) {
    nodeMap[imap.second] = new InstanceGraphNode(imap.second,false);
  }
  
  //populate all the nodes with pointers to the instances
  for (auto nodemap : nodeMap) {
    if (isa<Generator>(nodemap.first) || !nodemap.first->hasDef()) continue;
    ModuleDef* mdef = cast<Module>(nodemap.first)->getDef();
    for (auto instmap : mdef->getInstances()) {
      Instantiable* icheck = instmap.second->getInstantiableRef();
      if (nodeMap.count(icheck)==0) {
        auto node = new InstanceGraphNode(icheck,true);
        nodeMap[icheck] = node;
        //externalNodeMap[icheck] = node;
      }
      nodeMap[icheck]->addInstance(instmap.second,nodemap.second);
    }
  }

  for (auto imap : nodeMap) {
    sortVisit(imap.second);
  }
}



