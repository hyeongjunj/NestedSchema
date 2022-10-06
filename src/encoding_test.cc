#include "encoding.h"
#include "gtest/gtest.h"
using namespace nestedSchema;
/*
This TEST tests whether nested schema can be encoded and decoded properly.
In order to test this, we choose to encode certain nested schema and
decode it, store its byte-stream output and re-encoding it. 
If encoding and decoding works properly, 
those two byte-stream output should be the same.
*/
// Schema 1
/*
NestedExample 
field1: Struct {nf1: String, nf2: Struct { nnf1: Map<String, String> }} 
field2: Array<Struct {nf2: String, nf3: String }
*/
TEST(ENCODING_TEST_1, HANDLES_NESTED_TYPE_1) {
  Schema schema1;
  schema1.add_field("field1",
      STRUCT()->add("nf1", STRING("ABCDE"))
              ->add("nf2", STRUCT()->add("nnf1", MAP(STRING("ABC"), STRING("AB")))));
  schema1.add_field("field2",
      ARRAY("STRUCT")->add(STRUCT()->add("nf2", STRING("ABCDEFGHIJKLMNOPQ"))
                     ->add("nf3", STRING("ABCDEFGHIJ"))));
  Encoder e1,e2;
  Decoder d;
  Bytes bytes1 = e1.encode(schema1);
  Bytes bytes2 = e2.encode(d.decode(bytes1));
  EXPECT_EQ(bytes1, bytes2);
}

TEST(ENCODING_TEST_2, HANDLES_NESTED_TYPE_2) {
