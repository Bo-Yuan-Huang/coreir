###How to read this:
Anything in <> should be filled in by user
Anything not in quotes is a json format defined below

```
//What is in the json file
{"top": [<namespaceName>, <moduleName>],
"namespaces":{<namespaceName>:Namespace, ...}
}

//Definitions

Namespace={
  "namedtypes": NamedType[]
  "namedtypegenerators":NamedTypeGenerators[]
  "modules":{<modulename>:Module,...},
  "generators":{<genname>:Generator, ...}
}

Type = ["BitIn"] | ["BitOut"] | ["Array", <N>, Type] | ["Record", [<key>, Type][]}] | ["NamedType", NamedTypeReference, GenArgs]
TypeArgs = "None" | [<N>,Type] |  | 

NamedTypeReference=[<namespaceName>,<typeName>]
NamedType = {"typename":<name>, "flippedtypename":<name>,"type":Type}
NamedTypeGenerators={"typename":<name>, "flippedtypename":<name>,"genParameter":GenParameter}

Module = {"type":Type,"configparams":GenParameter, "metadata":Metadata,"def":ModuleDef}

ModuleDef = "None" | {"metadata":Metadata,
"implementations":Metadata,
"instances": {<instname>:Instance,...}
"connections": Connection[]}

Generator = {"configparameters":GenParameters,"genparameters":GenParameters, "typegen":NamedTypeReference, "metadata":Metadata}

Instance = {"instref":InstantiatableReference, "args":GenArgs, "config":GenArgs, "metadata":Metadata}

InstantiatableReference = ["namespacename","InstantiatableName"]

Connection = [Wireable, Wireable, Metadata]

Wireable = {"metadata":Metadata, "path":[<topname>,[<a>,<b>,...]]}
     accesses topname.a.b. If "topname" is "self" then this is the module's interface.

GenParameters = "None" | {<key>:GenParameterType,...}
GenParameterType = "String" | "Uint" | "Int" | "Type" | "Instantiatable"

GenArgs = "None" | {<key>:GenArgsValue}
GenArgsValue = <String> | <Number> | Type | InstantiatableReference

Metadata={<key>:MetadataValue,...}
MetadataValue = <String> | <Number> (becomes double)
```
