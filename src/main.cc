#include "encoding.h"
#include "gtest/gtest.h"
using namespace nestedSchema;

int main() {
  
  Struct* a1 = new Struct();
  a1->Add2Struct("street", new Primitive("STRING", "1234"));
  a1->Add2Struct("city", new Primitive("STRING", "1"));
  a1->Add2Struct("zipcode", new Primitive("STRING", "123456"));

  Struct* s = new Struct();
  Struct* nested = new Struct();
  s->Add2Struct("nf1", new Primitive("STRING", "1234567"));
  nested->Add2Struct("nnf1", new Map(new Primitive("STRING", "1234"), 
                                     new Primitive("STRING", "123")));
  s->Add2Struct("nf2", nested);
  Struct* nested2 = new Struct();
  
  Array* arr = new Array();
  arr->Add2Array(new Primitive("STRING", "1234"));
  arr->Add2Array(new Primitive("STRING", "12341234"));
  arr->Add2Array(new Primitive("STRING", "123456"));
  nested2->Add2Struct("nnf1-1", new Map(arr, new Map(new Primitive("STRING", "12"), new Primitive("STRING", "12345"))));
  nested2->Add2Struct("nnf1-2", new Primitive("STRING", "123456789"));
  s->Add2Struct("nf3", nested2);

  Struct* s2 = new Struct();
  s2->Add2Struct("1", s);
  s2->Add2Struct("2", new Primitive("STRING", "last"));
  

  Struct* ss = new Struct();
  Struct* ns = new Struct();
  ns->Add2Struct("nf1", new Primitive("STRING", "1234567890"));
  ss->Add2Struct("f1", ns);
  ss->Add2Struct("f2", new Primitive("STRING", "123456789012345678901234567890"));

  Schema schema;
  //schema.AddElement("f1", new Primitive("STRING", "v1"););
  //schema.AddElement("f2", new Primitive("STRING", "v1-1"));
  //schema.AddElement("f3", a1); 
  Struct* decodeTest = new Struct();
  decodeTest->Add2Struct("f1", new Primitive("STRING", "1234567"));
  //schema.AddElement("f4", s);
  //schema.AddElement("f1", decodeTest);
  

  //std::cout<<"bytes[0] "<<bytes[0]<<"\n";
  //std::cout<<bytes.size()<<"\n";

  Struct *N = new Struct();
  Struct *N1 = new Struct();
  Struct *N2 = new Struct();
  N2->Add2Struct("nf2", new Primitive("STRING", "12"));
  N2->Add2Struct("nf3", new Primitive("STRING", "123"));
  Struct *N3 = new Struct();
  N3->Add2Struct("nf2", new Primitive("STRING", "43"));
  N3->Add2Struct("nf3", new Primitive("STRING", "53677"));
  N1->Add2Struct("nnf1", new Map(new Primitive("STRING", "abc"),new Primitive("STRING", "def")));
  N->Add2Struct("nf1", new Primitive("STRING", "1890"));
  N->Add2Struct("nf2", N1);
  Array *ar = new Array();
  ar->Add2Array(N2);
  ar->Add2Array(N3);
  Struct* NN = new Struct();
  NN->Add2Struct("field1", N);
  NN->Add2Struct("field2", ar);
  schema.AddElement("f4", NN);
  Encoder e,E,e1,E1;
  Decoder d;
  Schema schema2;
  schema2.AddElement("s2", s);

  //Schema sch("testschema");
  

  //Bytes bytes_1 = e.encode(schema2);
  //Bytes bytes_2 = E.encode(d.decode(bytes_1));
  Bytes bytes1 = E.encode(schema);
  Bytes bytes2 = e.encode(d.decode(bytes1));
  Bytes bytes1_1 = e1.encode(schema2);
  Bytes bytes2_2 = E1.encode(d.decode(bytes1_1));
  if(bytes1 == bytes2) std::cout<<"SUCCESS\n";
  if(bytes1_1 == bytes2_2) std::cout<<"SUCCESS\n";
/*
  Schema tg("Person");
*/
  return 0;
}