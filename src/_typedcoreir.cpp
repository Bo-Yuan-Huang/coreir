#ifndef TYPEDCOREIR_CPP_
#define TYPEDCOREIR_CPP_

#include "typeCache.hpp"
#include "enums.hpp"
#include "typedcoreir.hpp"
#include <cassert>
#include <vector>
#include <sstream>

using namespace std;

TypedModuleDef::TypedModuleDef(string name, Type* type) : ModuleDef(name,type,TMDEF) {
  delete interface;
  interface = new TypedInterface(this,Flip(type));
}


Instance* TypedModuleDef::addInstance(string instname,Instantiable* m, GenArgs* g) {
  if (!m->isKind(TMDEF)) {
    throw "Can only add a TypedModuleDef instnace";
  }
  TypedModuleDef* tm = (TypedModuleDef*) m;
  TypedInstance* tinst = new TypedInstance(this,tm->getType(),instname,tm);
  instances.push_back(tinst);
  return tinst;
}

bool hasInput(Type* t) {
  if (t->isBase()) 
    return (((BaseType*) t)->getDir() == IN);
  else if (t->isKind(ARRAY)) 
    return hasInput(((ArrayType*) t)->getElemType());
  else if (t->isKind(RECORD)) {
    for (auto field : ((RecordType*) t)->getRecord()) {
      if (hasInput(field.second)) return true;
    }
    return false;
  }
  else {
    throw "fuck";
  }
}

void TypedModuleDef::wire(Wireable* a, Wireable* b) {
  //make sure both wires are 'Types'
  ostringstream err;
  if (!a->isTyped() || !b->isTyped()) {
    throw "These wires are not Typed!";
  }

  //Make sure you are connecting within the same container
  if (a->getContainer()!=this || b->getContainer() != this) {
    err << "ERROR: Connections can only occur within the same module\n";
    err << "  This ModuleDef: "  << this->getName() << endl;
    err << "  Module of " << *a << ": " << a->getContainer()->getName() << endl;
    err << "  Module of " << *b << ": " << b->getContainer()->getName() << endl;
    throw err.str();
  }
  TypedWire* atwire = castTypedWire(a->wire);
  TypedWire* btwire = castTypedWire(b->wire);
  
  Type* aType = atwire->getType();
  Type* bType = btwire->getType();
  //Make sure the flip of the types match
  if (aType != Flip(bType)) {
    err << "ERROR: Cannot connect these two types\n";
    err << "  " << *a << ": " << *(atwire->getType()) << "\n";
    err << "  " << *b << ": " << *(btwire->getType()) << "\n";
    throw err.str();
  }
  bool aHasInput = hasInput(aType);
  bool bHasInput = hasInput(bType);
  //Make sure 'a' is not already wired (if has inputs)
  if (aHasInput && atwire->isWired()) {
    err << "ERROR: " << *a << " has inputs and is already connected!\n";
    throw err.str();
  }
  //Make sure 'b' is not already wired (if has inputs)
  if (bHasInput && btwire->isWired()) {
    err << "ERROR: " << *b << " has inputs and is already connected!\n";
    throw err.str();
  }
  //Make sure 'a' does not have children alreay wired (that are inputs)
  if (atwire->hasChildrenWired()) {
    err << "ERROR: " << *a << "has children(inputs) already connected!\n";
    throw err.str();
  }
  if (btwire->hasChildrenWired()) {
    err << "ERROR: " << *b << "has children(inputs) already connected!\n";
    throw err.str();
  }

  //Update 'a' and 'b' (and children)
  atwire->addWiring(btwire);
  btwire->addWiring(atwire);
  
  //Update parents if we are wiring inputs.
  //TODO Confusing names. But this is setting the _childrenWired flag of the parents
  //  and NOT setting the _wired of the children lol
  if (aHasInput) {
    atwire->setChildrenWired(); 
  }
  if (bHasInput) {
    btwire->setChildrenWired();
  }

  wirings.push_back({a,b});
}

//Set _wired and all children's parentWired
void TypedWire::addWiring(Wire* w) {
  _wired = true;
  for (auto child : children) {
    TypedWire* tchild = castTypedWire(child.second);
    tchild->setParentWired();
  }
  wirings.push_back(w);
}

//Set _parentWired and all children's setParentWired
void TypedWire::setParentWired() {
  _parentWired = true;
  for (auto child : children) {
    TypedWire* tchild = castTypedWire(child.second);
    tchild->setParentWired();
  }
}

void TypedWire::setChildrenWired() {
  _childrenWired=true; 
  if (parent) {
    TypedWire* tparent = castTypedWire(parent);
    tparent->setChildrenWired();
  }
}


//string wireDebug(TypedWire* w) {
//  cout ""w->from
//}

//TODO make sure there exists at least 1 children given that _childrenWired is set
//TODO This definitely needs nice error messages
void TypedWire::checkWired() {
  if (_wired) return;
  if (type->isBase()) throw (from->toString() + " Is not connected to anything!");
  
  //Should have children...
  //Check if all entries of map exist and are wired
  //Have to deal with Records and Arrays differently
  if(type->isKind(RECORD)) {
    //iterate over type record keys
    auto record = ((RecordType*)type)->getRecord();
    for (auto tit : record) {
      auto it = children.find(tit.first);
      if (it==children.end()) throw (from->toString() + "." + tit.first + " Is not connected!");
      TypedWire* child = castTypedWire(it->second);
      child->checkWired();
    }
  } 
  else {
    // iterate over the array
    for (uint i=0; i<((ArrayType*)type)->getLen(); ++i) {
      auto it = children.find(to_string(i));
      if (it==children.end()) throw (from->toString() + "." + to_string(i) + " Is not connected!");;
      TypedWire* child = castTypedWire(it->second);
      child->checkWired();
    }
  }
  return;
}

// TODO hack I need to do to get around diamond inheritance
// Needs to handle numbers and records
Select* selprotoype(TypedModuleDef* container, Wireable* parent, Type* type, string selStr) {
  ostringstream err;
  Type* selType;
  if (type->isKind(RECORD)) {
    selType = ((RecordType*)type)->sel(selStr);
    if (!selType) {
      err << "Bad Select: \'" << selStr << "\' not found" <<endl;
      err << "  Module: " << *container << endl;
      err << "  Wireable: " << *parent << endl;
      err << "  Type: " << *type << endl;
      throw err.str();
    }
  }
  else if (type->isKind(ARRAY)) {
    if (!isNumber(selStr)) {
      err << "Bad Select: \'" << selStr << "\' is not a number" << endl;
      err << "  Module: " << *container << endl;
      err << "  Wireable: " << *parent << endl;
      err << "  Type: " << *type << endl;
      throw err.str();
    }
    ArrayType* atype = (ArrayType*)type; 
    selType = ((ArrayType*)atype)->sel(std::stoi(selStr));
    if (!selType) {
      err << "Bad Select: \'" << selStr << "\' is not in range [0:" << (atype->getLen()-1) << "]" << endl;
      err << "  Module: " << *container << endl;
      err << "  Wireable: " << *parent << endl;
      err << "  Type: " << *type << endl;
      throw err.str();
    }
  }
  else {
    err << "Bad Select: Trying to select \'" << selStr << "\' From base type" << endl;
    err << "  Module: " << *container << endl;
    err << "  Wireable: " << *parent << endl;
    err << "  Type: " << *type << endl;
    throw err.str();
  }
  return container->getCache()->newTypedSelect(container,parent,selType,selStr);
}

Select* TypedInterface::sel(string s) {
  return selprotoype((TypedModuleDef*)container,this,castTypedWire(wire)->getType(),s);
}

Select* TypedInstance::sel(string s) {
  return selprotoype((TypedModuleDef*)container,this,castTypedWire(wire)->getType(),s);
}

Select* TypedSelect::sel(string s) {
  return selprotoype((TypedModuleDef*)container,this,castTypedWire(wire)->getType(),s);
}



#endif //TYPEDCOREIR_CPP_
