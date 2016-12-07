#ifndef COREIR_HPP_
#define COREIR_HPP_


#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cassert>
#include "types.hpp"
#include "enums.hpp"

using namespace std;


//So much mutual definition, so forward declare
class WireableCache;
class Wireable;
class Interface;
class Instance;
class Select;


class Instantiable {
  protected:
    InstantiableEnum instantiableType;
    string name;
    string nameSpace;
  public :
    Instantiable(InstantiableEnum instantiableType, string name,string nameSpace) : instantiableType(instantiableType), name(name), nameSpace(nameSpace) {}
    virtual ~Instantiable() {};
    string getName() { return name;}
    virtual string toString() const =0;
};

std::ostream& operator<<(ostream& os, const Instantiable&);

class GeneratorDecl : public Instantiable {
  
  public :
    GeneratorDecl(string name,string nameSpace) : Instantiable(GDEC,name,nameSpace) {}
    virtual ~GeneratorDecl() {}
    string toString() const {
      return "GeneratorDecl: Namespace:"+nameSpace+" name:"+name;
    }

};

//class Generator : GeneratorDecl {
//  void* genParams
//  public :
//    Generator(string name,string nameSpace,void* genParams) : GeneratorDecl(GEN,name,nameSpace), genParams(genParams) {}
//
//}

class ModuleDecl : public Instantiable {
  
  public :
    ModuleDecl(string name,string nameSpace) : Instantiable(MDEC,name,nameSpace) {}
    virtual ~ModuleDecl() {}
    string toString() const {
      return "ModuleDecl: Namespace:"+nameSpace+" name:"+name;
    }
};

struct simfunctions_t {
  //void* iface,void* state,void* dirty,void* genargs)
  void (*updateOutput)(void*,void*,void*,void*);
  void* (*allocateState)(void);
  void (*updateState)(void*,void*,void*,void*);
  void (*deallocateState)(void*);
};

typedef std::pair<Wireable*,Wireable*> Connection ;

class ModuleDef : public Instantiable {
  Type* type;
  Interface* interface; 
  vector<Instance*> instances; // Should it be a map?
  vector<Connection> connections;
  string verilog;
  simfunctions_t sim;
  WireableCache* cache;
  public :
    ModuleDef(string name, string nameSpace, Type* type);
    virtual ~ModuleDef();
    void print(void);
    void addVerilog(string _v) {verilog = _v;}
    void addSimfunctions(simfunctions_t _s) {sim = _s;}
    WireableCache* getCache(void);
    Instance* addInstance(string,ModuleDef*);
    Instance* addInstance(string,ModuleDecl*);
    Instance* addInstance(string,GeneratorDecl*, Type*, void*);
    Interface* getInterface(void) {return interface;}
    void Connect(Wireable* a, Wireable* b);
    vector<Instance*> getInstances(void) { return instances;}
    vector<Connection> getConnections(void) { return connections; }
    string toString() const {
      return "module: should probably use print()";
    }
};



class Wireable {
  protected :
    WireableEnum wireableType;
    ModuleDef* container; // ModuleDef which it is contained in 
    vector<Wireable*> connections; 
    map<string,Wireable*> children;
  public : 
    Wireable(WireableEnum wireableType, ModuleDef* container) : wireableType(wireableType),  container(container) {}
    ~Wireable() {}
    ModuleDef* getContainer() { return container;}
    void addChild(string selStr, Wireable* wb);
    void addConnection(Wireable* w);
    Select* sel(string);
    Select* sel(uint);
    virtual string toString() const=0;
  private :
    friend ostream& operator<<(ostream&, const Wireable&);
};


class Interface : public Wireable {
  public :
    Interface(ModuleDef* container) : Wireable(IFACE,container) {}
    virtual ~Interface() {}
    string toString() const;
};

class Select : public Wireable {
  Wireable* parent;
  string selStr;
  public :
    Select(ModuleDef* container, Wireable* parent, string selStr) : Wireable(SEL,container), parent(parent), selStr(selStr) {}
    virtual ~Select() {}
    Wireable* getParent() { return parent; }
    string getSelStr() { return selStr; }
    string toString() const;
};


//TODO potentially separate out generator instances and module instances
class Instance : public Wireable {
  string name;
  Instantiable* instantiableType;
  
  public :
    Instance(ModuleDef* container, string name, Instantiable* instantiableType) : Wireable(INST,container), name(name), instantiableType(instantiableType) {}
    virtual ~Instance() {}
    Instantiable* getInstantiableType() {return instantiableType;}
    string getName() { return name; }
    string toString() const;
};

void* allocateFromType(Type* t);
void deallocateFromType(Type* t, void* d);

class GenInstance : public Instance {
  Type* genParamsType;
  void* genParams;
  public :
    GenInstance(ModuleDef* container, string name, Instantiable* instantiableType,Type* genParamsType, void* genParams) : Instance(container,name,instantiableType), genParamsType(genParamsType), genParams(genParams) {}
    virtual ~GenInstance() {deallocateFromType(genParamsType,genParams);}
 
};


typedef std::pair<Wireable*, string> SelectParamType;
class WireableCache {
  map<SelectParamType,Select*> SelectCache;
  public :
    WireableCache() {};
    ~WireableCache();
    Select* newSelect(ModuleDef* container, Wireable* parent, string sel);
};


class CoreIRContext;
class NameSpace;

typedef ModuleDef* (*genfun_t)(NameSpace*,void*);


class NameSpace {
  string name;
  map<string,ModuleDef*> modList;
  map<string,genfun_t> genList;
  public :
    NameSpace(string name) : name(name) {}
    ~NameSpace();
    string getName() { return name;}
    ModuleDef* defineModuleDef(string name, Type* type);
    void defineGenerator(string name,genfun_t);

    // Note: ModuleDef m will be deleted by this namesace's destructor
    void addDefinedModuleDef(string name, ModuleDef* m);
};



class CoreIRContext {
  NameSpace* global;
  map<string,NameSpace*> libs;
  vector<Instantiable*> opaques;
  public :
    CoreIRContext();
    ~CoreIRContext();
    NameSpace* getGlobal() {return global;}
    NameSpace* registerLib(string name);
    ModuleDecl* declareMod(string nameSpace, string name);
    GeneratorDecl* declareGen(string nameSpace,string name);
};

CoreIRContext* newContext();
void deleteContext(CoreIRContext* m);




/////
string WireableEnum2Str(WireableEnum wb);
bool Validate(Instantiable* c);

// Int Type functions
IntType* Int(uint bits, Dir dir);
IntType* Int(uint bits);
//Type* Float(uint ebits, uint mbits, Dir dir);
//Type* Bool(Dir dir);
ArrayType* Array(Type* elemType, uint len);
RecordType* Record(recordparam_t record);
Type* Sel(Type* record, string key);
Type* Flip(Type*);
Type* In(Type*);
Type* Out(Type*);

void compile2Verilog(ModuleDef* m);

typedef struct dirty_t {
  uint8_t dirty;
} dirty_t;

uint8_t isDirty(dirty_t* d);
void setDirty(dirty_t* d);



#endif //COREIR_HPP_
