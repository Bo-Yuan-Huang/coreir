#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include "namespace.hpp"
#include "typecache.hpp"
#include "types.hpp"
#include "error.hpp"
#include <string>
#include "common.hpp"
#include <unordered_set>
#include <vector>

using namespace std;
class Context {
  Namespace* global;
  map<string,Namespace*> libs;
  
  uint maxErrors;
  vector<Error> errors;
 
  //Memory management
  TypeCache* cache;
  vector<GenArg*> genargList;
  vector<GenArgs*> genargsList;
  
  public :
    Context();
    ~Context();
    Namespace* getGlobal() {return global;}
    
    //Error functions
    void error(Error e) { 
      errors.push_back(e);
      if (e.isfatal || errors.size() >= maxErrors) die();
    }
    bool haserror() { return errors.size()>0; }
    void checkerrors() { if (haserror()) die(); }
    void die() { 
      printerrors();
      delete this; // sketch but okay if exits I guess
      cout << "I AM DYING!" << endl;
      exit(1);
    }
    void printerrors() { 
      for (auto err : errors) cout << "ERROR: " << err.toString() << endl << endl;
    }
    
    bool linkLib(Namespace* defns, Namespace* declns);
    
    Namespace* newNamespace(string name);
    Namespace* getNamespace(string s);

    Type* Any();
    Type* BitIn();
    Type* BitOut();
    Type* Array(uint n, Type* t);
    Type* Record(RecordParams rp);
    Type* TypeGenInst(TypeGen* tgd, GenArgs* args);

    GenArg* GInt(int i);
    GenArg* GString(string s);
    GenArg* GType(Type* t);
    int toInt(GenArg* g);
    string toString(GenArg* g);
    Type* toType(GenArg* g);
    GenArgs* newGenArgs(unordered_map<string,GenArg*> args);   

  
    Type* Flip(Type* t);



};

Module* loadModule(Context* c, string filename);

Context* newContext();
void deleteContext(Context* m);


#endif //CONTEXT_HPP_
