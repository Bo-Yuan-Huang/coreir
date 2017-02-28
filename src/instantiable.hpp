#ifndef COREIR_HPP_
#define COREIR_HPP_


#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <cassert>

#include "types.hpp"
#include "enums.hpp"
#include "genargs.hpp"

using namespace std;


//So much mutual definition, so forward declare
class SelCache;
class Wireable;
class Interface;
class Instance;
class Select;


class CoreIRContext;
class Instantiable {
  protected:
    InstantiableKind kind;
    CoreIRContext* context;
    string nameSpace;
    string name;
  public :
    Instantiable(InstantiableKind kind, CoreIRContext* context,string nameSpace, string name) : kind(kind), context(context), nameSpace(nameSpace), name(name) {}
    virtual ~Instantiable() {};
    virtual string toString() const =0;
    CoreIRContext* getContext() { return context;}
    string getName() { return name;}
    string getNameSpaceStr() { return nameSpace;}
    void setNameSpaceStr(string _n) {nameSpace = _n;}
    //string getQualifiedName() { return (nameSpace.empty() ? "this" : nameSpace)  + "." + name; }
    bool isKind(InstantiableKind t) {return kind==t;}
};

std::ostream& operator<<(ostream& os, const Instantiable&);


class Generator : public Instantiable {
  ArgKinds argkinds;
  TypeGen* typegen;
  genFun genfun;
  public :
    Generator(CoreIRContext* c,string name,ArgKinds argkinds, TypeGen* typegen);
    string toString() const {
      return "Generator: " + name;
    }
    TypeGen* getTypeGen() { return typegen;}
    void addGeneratorDef(genFun fun) { genfun = fun;}
    //genargs_t getGentypes(void) {return gentypes;}
    //genfun_t getGenfun(void) {return genfun;}
};

class Module : public Instantiable {
  Type* type;
  ModuleDef* def;
  string verilog;
  public :
    Module(CoreIRContext* c,string name, Type* type) : Instantiable(MOD,c,"",name), type(type) {}
    string toString() const;
    Type* getType() { return type;}
    void addModuleDef(ModuleDef* _def) { def = _def;}
    ModuleDef* newModuleDef();
    void print(void);
    //TODO turn this into metadata
    void addVerilog(string _v) {verilog = _v;}
};

typedef std::pair<Wireable*,Wireable*> Wiring ;
class ModuleDef {
  
  protected:
    Module* module;
    Interface* interface; 
    set<Instance*> instances; // Should it be a map?
    set<Wiring> wirings;
    SelCache* cache;

  public :
    ModuleDef(Module* m);
    ~ModuleDef();
    //SelCache* getCache(void) { return cache;}
    set<Instance*> getInstances(void) { return instances;}
    set<Wiring> getWirings(void) { return wirings; }
    bool hasInstances(void) { return !instances.empty();}
    void print(void);
    CoreIRContext* getContext() { return module->getContext(); }
    string getName() {return module->getName();}
    Type* getType() {return module->getType();}
    SelCache* getCache() { return cache;}
    Instance* addInstanceGenerator(string,Generator*, GenArgs*);
    Instance* addInstanceModule(string,Module*);
    Interface* getInterface(void) {return interface;}
    void wire(Wireable* a, Wireable* b);
    
};

class Wireable {
  protected :
    WireableKind kind;
    ModuleDef* moduledef; // ModuleDef which it is contained in 
    Type* type;
    set<Wireable*> wirings; 
    map<string,Wireable*> children;
  public :
    Wireable(WireableKind kind, ModuleDef* moduledef, Type* type) : kind(kind),  moduledef(moduledef), type(type) {}
    virtual ~Wireable() {}
    virtual string toString() const=0;
    ModuleDef* getModuleDef() { return moduledef;}
    CoreIRContext* getContext() { return moduledef->getContext();}
    bool isKind(WireableKind e) { return e==kind;}
    WireableKind getKind() { return kind ; }
    Type* getType() { return type;}
    void ptype() {cout << "Kind=" <<wireableKind2Str(kind);}
    void addWiring(Wireable* w) { wirings.insert(w); }
    void addChild(string selStr, Wireable* w) { children.emplace(selStr,w); }
    
    Select* sel(string);
    Select* sel(uint);


};

ostream& operator<<(ostream&, const Wireable&);

class Interface : public Wireable {
  public :
    Interface(ModuleDef* context,Type* type) : Wireable(IFACE,context,type) {};
    string toString() const;
};

//TODO potentially separate out moduleGen instances and module instances
class Instance : public Wireable {
  string instname;
  Instantiable* instRef;
  
  GenArgs* genargs;
 
  public :
    Instance(ModuleDef* context, string instname, Instantiable* instRef,Type* type, GenArgs* genargs =nullptr)  : Wireable(INST,context,type), instname(instname), instRef(instRef), genargs(genargs) {}
    ~Instance();
    string toString() const;
    Instantiable* getInstRef() {return instRef;}
    string getInstname() { return instname; }
    GenArgs* getGenArgs() {return genargs;}
    //void replace(Instantiable* newRef) { instRef = newRef;}
};

class Select : public Wireable {
  protected :
    Wireable* parent;
    string selStr;
  public :
    Select(ModuleDef* context, Wireable* parent, string selStr, Type* type) : Wireable(SEL,context,type), parent(parent), selStr(selStr) {
        //parent->wire->addChild(selStr,wire);
    }
    string toString() const;
    Wireable* getParent() { return parent; }
    string getSelStr() { return selStr; }
};


typedef std::pair<Wireable*, string> SelectParamType;
class SelCache {
  map<SelectParamType,Select*> cache;
  public :
    SelCache() {};
    ~SelCache();
    Select* newSelect(ModuleDef* context, Wireable* parent, string selStr,Type* t);
};


//void* allocateFromType(Type* t);
//void deallocateFromType(Type* t, void* d);


// Compiling functions.
// resolve, typecheck, and validate will throw errors (for now)

// For now, these functions mutate m. TODO (bad compiler practice probably)

// This is the resolves the Decls and runs the moduleGens
void resolve(CoreIRContext* c, ModuleDef* m);

//Only resolves the Decls
void resolveDecls(CoreIRContext* c, ModuleDef* m);

//Only runs the moduleGens
void runGenerators(CoreIRContext* c, ModuleDef* m);


// This 'typechecks' everything
  //   Verifies all selects are valid
  //   Verifies all connections are valid. type <==> FLIP(type)
  //   Verifies inputs are only connected once

//typedef map<ModuleDef*,TypedModuleDef*> typechecked_t;
//typechecked_t* typecheck(CoreIRContext* c, ModuleDef* m);


// This verifies that there are no unconnected wires
//void validate(TypedModuleDef* tm);

// This generates verilog
//string verilog(TypedModuleDef* tm);

// Convieniance that runs resolve, typecheck and validate
// and catches errors;
void compile(CoreIRContext* c, ModuleDef* m, fstream* f);

#endif //COREIR_HPP_
