#ifndef TYPES_CPP_
#define TYPES_CPP_

#include "types.hpp"

namespace CoreIR {

void Type::print(void) { cout << "Type: " << (*this) << endl; }

bool Type::sel(Context* c,string sel, Type** ret, Error* e) {
  *ret = c->Any(); 
  e->message("Cant select from this type!");
  e->message("  Type: " + toString());
  return true;
}

std::ostream& operator<<(ostream& os, const Type& t) {
  os << t.toString();
  return os;
}

string RecordType::toString(void) const {
  string ret = "{";
  uint len = record.size();
  uint i=0;
  for(auto sel : _order) {
    ret += "'" + sel + "':" + record.at(sel)->toString();
    ret += (i==len-1) ? "}" : ", ";
    ++i;
  }
  return ret;
}
TypeGenType::TypeGenType(TypeGen* def, Args* args) : Type(TYPEGEN), def(def), args(args) {
  assert(args->checkParams(def->genparams));
}

bool AnyType::sel(Context* c, string sel, Type** ret, Error* e) {
  *ret = c->Any();
  return false;
}
bool TypeGenType::sel(Context* c, string sel, Type** ret, Error* e) {
  *ret = c->Any();
  return false;
}

bool ArrayType::sel(Context* c, string sel, Type** ret, Error* e) {
  *ret = c->Any();
  if (!isNumber(sel)) {
    e->message("Idx into Array needs to be a number");
    e->message("  Idx: '" + sel + "'");
    e->message("  Type: " + toString());
    return true;
  }
  uint i = stoi(sel);
  if(i >= len ) {
    e->message("Index out of bounds for Array");
    e->message("  Required range: [0," + to_string(len-1) + "]");
    e->message("  Idx: " + sel);
    e->message("  Type: " + toString());
    return true;
  }
  *ret = elemType;
  return false;
}
 
// TODO should this actually return Any if it is missing?
bool RecordType::sel(Context* c, string sel, Type** ret, Error* e) {
  *ret = c->Any();
  auto it = record.find(sel);
  if (it != record.end()) {
    *ret = it->second;
    return false;
  } 
  e->message("Bad select field for Record");
  e->message("  Sel: '" + sel + "'");
  e->message("  Type: " + toString());
  return true;

}

}//CoreIR namespace

#endif //TYPES_CPP_
