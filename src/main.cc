#include "encoding.h"
  std::ostream& operator<< (std::ostream& os, std::byte b) {
    return os << std::bitset<8>(std::to_integer<int>(b));
  }
int main() {
  Struct* a1 = new Struct();
  a1->Add2Struct("street", new Primitive("STRING", "1234"));
  a1->Add2Struct("city", new Primitive("STRING", "1"));
  a1->Add2Struct("zipcode", new Primitive("STRING", "123456"));

  Struct* s = new Struct();
  Struct* nested = new Struct();
  s->Add2Struct("nf1", new Primitive("STRING", "1234567"));
  nested->Add2Struct("nnf1", new Map(new Primitive("STRING", "1234"), new Primitive("STRING", "123")));
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
  
  /*
  Struct
    nf1 string(v5)
    nf2 Struct{nnf1 : Map(v6,v7)}
    nf3 Struct{v8, Map(v9, arr(a1,a2,a3))}
  */
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
  schema.AddElement("f4", s);
  //schema.AddElement("f1", decodeTest);
  Encoder e;
  Decoder d;
  Bytes bytes = e.encode(schema);

  std::cout<<"bytes[0] "<<bytes[1]<<"\n";
  //std::cout<<bytes.size()<<"\n";
  d.decode(bytes);
  
  return 0;
}
