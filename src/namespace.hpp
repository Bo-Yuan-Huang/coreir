#ifndef NAMESPACE_HPP_
#define NAMESPACE_HPP_

#include <string>
#include <map>
#include "types.hpp" // For TypeGen
#include "instantiable.hpp"
#include "enums.hpp"

using namespace std;

class Namespace {
  CoreIRContext* c;
  string name;

  map<string,Module*> mList;
  map<string,Generator*> gList;
  map<string,TypeGen*> tList;
  
  public :
    Namespace(CoreIRContext* c, string name) : c(c), name(name) {}
    ~Namespace();
    string getName() { return name;}
    TypeGen* newTypeGen(string name, string nameFlipped, ArgKinds kinds, TypeGenFun fun);
    TypeGen* getTypeGen(string name) {
      assert(hasTypeGen(name));
      return tList.find(name)->second;
    }
    bool hasTypeGen(string name) {
      return tList.find(name) != tList.end();
    }
    
    void addModule(Module* i);
    void addGenerator(Generator* i);

    Generator* getGenerator(string gname);
    Module* getModule(string mname);

   
    void print() {
      cout << "Namespace: " << name << endl;
      cout << "  NYI" << endl;
      //cout << "  Instantiables:" << endl;
      //for (auto it : iList) cout << "    " << it.second->toString() <<endl;
      cout << endl;
    }

};

Namespace* newNamespace(CoreIRContext* c,string name);


#endif //NAMESPACE_HPP_
