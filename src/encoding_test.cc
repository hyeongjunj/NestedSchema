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
/*
  NestedExample  
  field1: Struct {nf1: String, nf2: Struct { nnf1: Map<String, String> }} 
  field2: Array<Struct {nf2: String, nf3: String }
  */
 
TEST(ENCODING_TEST_1, HANDLES_NESTED_TYPE_1) {
  
  Schema schema("NextedExample");
  schema.add_field("field1",
      STRUCT()->add("nf1", STRING("1234"))
              ->add("nf2", STRUCT()->add("nnf1", MAP(STRING("1234"), STRING("1234")))));
  schema.add_field("field2",
      ARRAY("STRUCT")->add(STRUCT()->add("nf2", STRING("1234"))
                                   ->add("nf3", STRING("1234")))
                     ->add(STRUCT()->add("nf2", STRING("1234"))
                                   ->add("nf3", STRING("1234")))
                     ->add(STRUCT()->add("nf2", STRING("1234"))
                                   ->add("nf3", STRING("1234"))));
  Encoder e1,e2;
  Decoder d;
  Bytes bytes1 = e1.encode(schema);
  Bytes bytes2 = e2.encode(d.decode(bytes1));
  EXPECT_EQ(bytes1, bytes2);
}

/*
Person {
first_name: String,
last_name: String,
age: i8
salary: i32,
address: Struct {street: String, 
                 city: String, 
                 state: String, 
                 zip: String } 
properties: Map<String, String>
assets: Array<String> 
}
*/

TEST(ENCODING_TEST_2, HANDLES_NESTED_TYPE_2) {
  Schema schema("Person");
  schema.add_field("first name", STRING("HYEON JEON")); 
  schema.add_field("last name", STRING("JEON")); 
  schema.add_field("age", I8("27")); 
  schema.add_field("salay", I32("0"));
  schema.add_field("address", STRUCT()->add("street", STRING("12323456"))
                                      ->add("city", STRING("SEOUL"))
                                      ->add("country", STRING("Republ icn")));
  schema.add_field("properties", MAP(STRING("IHAVE"),STRING("NOTHING")));
  schema.add_field("assets", ARRAY("STRING")->add(STRING("HOUSE"))
                                            ->add(STRING("CAR"))
                                            ->add(STRING("PRIVATE JET")));
  Encoder e1,e2;
  Decoder d;
  Bytes bytes1 = e1.encode(schema);
  Bytes bytes2 = e2.encode(d.decode(bytes1));
  EXPECT_EQ(bytes1, bytes2);
}  

TEST(ENCODING_TEST_3, HANDLES_NESTED_TYPE_3) {
  Schema schema("TEST3");
  schema.add_field("field1", STRING("12345678"));
  schema.add_field("field2", STRUCT()->add("nf1", MAP(STRING("FED"), STRING("CKOE"))));
  schema.add_field("field3", STRUCT()->add("nf2", MAP(ARRAY("STRING")->add(STRING("DSAD"))
                                                                     ->add(STRING("DSAV"))
                                                                     ->add(STRING("DSADS")),
                                                      MAP(STRING("BBB"), STRING("CDA"))))
                                     ->add("nf3", STRING("DSADASFEQG")));
  Encoder e1,e2;
  Decoder d;
  Bytes bytes1 = e1.encode(schema);
  Bytes bytes2 = e2.encode(d.decode(bytes1));
  EXPECT_EQ(bytes1, bytes2);
}  
