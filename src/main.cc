#include "encoding.h"

int main() {
  Struct* a1 = new Struct();
  a1->Add2Struct("street", new Primitive("STRING", "v2"));
  a1->Add2Struct("city", new Primitive("STRING", "v3"));
  a1->Add2Struct("zipcode", new Primitive("STRING", "v4"));

  Struct* s = new Struct();
  Struct* nested = new Struct();
  s->Add2Struct("nf1", new Primitive("STRING", "v5"));
  nested->Add2Struct("nnf1", new Map(new Primitive("STRING", "v6"), new Primitive("STRING", "v7")));
  s->Add2Struct("nf2", nested);
  Struct* nested2 = new Struct();
  nested2->Add2Struct("nnf1-1", new Primitive("STRING", "v8"));
  Array* arr = new Array();
  arr->Add2Array(new Primitive("STRING", "a1"));
  arr->Add2Array(new Primitive("STRING", "a2"));
  arr->Add2Array(new Primitive("STRING", "a3"));
  nested2->Add2Struct("nnf1-2", new Map(new Primitive("STRING", "v9"), new Map(new Primitive("STRING", "vv6"), new Primitive("STRING", "vv7"))));
  s->Add2Struct("nf3", nested2);
  
  /*
  Struct
    nf1 string(v5)
    nf2 Struct{nnf1 : Map(v6,v7)}
    nf3 Struct{v8, Map(v9, arr(a1,a2,a3))}
  */


  Schema schema;
  //schema.AddElement("f1", new Primitive("STRING", "v1"););
  //schema.AddElement("f2", new Primitive("STRING", "v1-1"));
  //schema.AddElement("f3", a1);
  
  schema.AddElement("f4", s);

  Encoder e;

  e.encode(schema);
  
  return 0;
}
