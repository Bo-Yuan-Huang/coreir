#include "fuzzing.hpp"

#include "../src/simulator/sim.hpp"
#include "../src/simulator/print_c.hpp"

#include <cstdlib>

namespace CoreIR {

  std::string ln(const std::string& s) {
    return s + ";\n";
  }

  std::string primitiveRandomInputString(CoreIR::Type& t, const std::string& var) {
    assert(isPrimitiveType(t));

    string val = to_string(rand() % 100);

    return ln(var + " = " + val);
  }

  std::string randomInputString(CoreIR::Type& tp, const std::string& var) {
    if (isPrimitiveType(tp)) {
      return primitiveRandomInputString(tp, var);
    }

    if (isArray(tp)) {

      ArrayType& tArr = static_cast<ArrayType&>(tp);
      Type& underlying = *(tArr.getElemType());

      string res = "";
      for (uint i = 0; i < tArr.getLen(); i++) {
	res += randomInputString(underlying, var + "[ " + to_string(i) + " ]");
      }

      //cArrayTypeDecl(underlying, varName + "[ " + std::to_string(tArr.getLen()) + " ]");
      return res;
      
    }

    assert(false);
      //assert(false);
  }

  std::string randomSimInputString(Module* mod) {
    auto args = simInputs(*mod);

    string res = "";
    for (auto& arg : args) {
      res += randomInputString(*(arg.first), arg.second);
    }

    return res;
  }

  std::string declareInputs(Module& mod) {
    string res;

    auto args = simInputs(mod);

    for (auto& arg : args) {
      res += ln(cArrayTypeDecl(*(arg.first), arg.second));
    }

    return res;
  }

  std::string randomSimInputHarness(Module* mod) {
    string res = "#include <stdint.h>\n";
    res += "#include <iostream>\n\n";
    res += "int main() {\n";

    res += declareInputs(*mod);

    
    res += randomSimInputString(mod);
    res += "}\n";

    return res;
  }

}
