#include "fuzzing.hpp"

#include "../src/simulator/algorithm.hpp"
#include "../src/simulator/sim.hpp"
#include "../src/simulator/print_c.hpp"
#include "../src/simulator/output.hpp"

#include <fstream>
#include <iostream>

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

  std::vector<std::pair<CoreIR::Type*, std::string> >
  simOutputVarDecls(Module& mod) {
    Type* tp = mod.getType();

    assert(tp->getKind() == Type::TK_Record);

    RecordType* modRec = static_cast<RecordType*>(tp);
    vector<pair<Type*, string>> declStrs;

    for (auto& name_type_pair : modRec->getRecord()) {
      Type* tp = name_type_pair.second;

      if (!tp->isInput()) {
	assert(tp->isOutput());

	declStrs.push_back({tp, name_type_pair.first});
      }
    }

    return declStrs;
  }
  
  std::string declareOutputs(Module& mod) {
    string res;

    auto args = simOutputVarDecls(mod);

    for (auto& arg : args) {
      res += ln(cArrayTypeDecl(*(arg.first), arg.second));
    }

    return res;
  }

  std::string callSimulate(Module& mod) {
    vector<string> args;

    for (auto& arg : simOutputVarDecls(mod)) {
      args.push_back("&" + arg.second);
    }

    vector<pair<Type*, string> > decls = simInputs(mod);

    sort_lt(decls, [](const pair<Type*, string>& tpp) {
	return tpp.second;
      });

    for (auto& arg : decls) {
      args.push_back(arg.second);
    }

    return ln("simulate( " + commaSepList(args) + " )");
  }
  
  std::string randomSimInputHarness(Module* mod) {
    string res = "#include <stdint.h>\n";
    res += "#include \"many_ops.h\"\n\n";
    res += "#include <iostream>\n\n";
    res += "int main() {\n";

    res += declareInputs(*mod);
    res += declareOutputs(*mod);

    res += randomSimInputString(mod);

    res += callSimulate(*mod);

    res += "}\n";

    return res;
  }

  int generateHarnessAndRun(const std::deque<vdisc>& topoOrder,
			    NGraph& g,
			    Module* mod,
			    const std::string& outFileBase,
			    const std::string& harnessFile) {

    string hFile = outFileBase + ".h";
    string codeFile = outFileBase + ".cpp";

    writeFiles(topoOrder, g, mod, codeFile, hFile);

    std::string harnessCode = randomSimInputHarness(mod);
    std::ofstream out(harnessFile);
    out << harnessCode;
    out.close();
    
    string runCmd = "clang++ " + codeFile + " " + harnessFile;
    int s = system(runCmd.c_str());

    cout << "Command result = " << s << endl;

    string runTest = "./a.out";
    int r = system(runTest.c_str());

    cout << "Test result = " << r << endl;

    return s || r;

    return 0;
  }
  
}
