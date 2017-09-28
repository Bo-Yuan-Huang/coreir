#ifndef COREIR_COMMON_HPP_
#define COREIR_COMMON_HPP_

#include "fwd_declare.h"

namespace CoreIR {

class ConnectionComp {
  public:
    static bool SPComp(const SelectPath& l, const SelectPath& r);
    bool operator() (const Connection& l, const Connection& r);
};

//TODO Ugly hack to create a sorted connection. Should make my own connection class
Connection connectionCtor(Wireable* a, Wireable* b);


//These are defined in helpers
bool isNumber(std::string s);
bool isPower2(uint n);
std::string Params2Str(Params);
std::string Values2Str(Values);
std::string SelectPath2Str(SelectPath path);
std::string Connection2Str(Connection con);

//Will call assertions
void checkValuesAreParams(Values args, Params params);

bool hasChar(const std::string s, char c);


template<class T> std::string toString(const T& t) {
  std::ostringstream stream;
  stream << t;
  return stream.str();
}

template<typename BackInserter>
BackInserter splitString(const std::string &s, char delim) {
    BackInserter elems;
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
}
template <class T, class A>
T join(const A &begin, const A &end, const T &t) {
  T result;
  for (A it=begin; it!=end; it++) {
    if (!result.empty())
      result.append(t);
    result.append(*it);
  }
  return result;
}

std::vector<std::string> splitRef(std::string s);

static std::unordered_map<std::string,std::unordered_set<std::string>> opmap({
  {"unary",{"not","neg"}},
  {"unaryReduce",{"andr","orr","xorr"}},
  {"binary",{
    "and","or","xor",
    "shl","lshr","ashr",
    "mul",
    "udiv","urem",
    "sdiv","srem","smod"
  }},
  {"binaryReduce",{"eq",
    "slt","sgt","sle","sge",
    "ult","ugt","ule","uge"
  }},
  {"ternary",{"mux"}},
});

void mergeValues(Values& v0, Values v1);

}

#endif

