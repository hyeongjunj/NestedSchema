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
  
  Schema schema("NestedExample");
  schema.add_field("field1",
      STRUCT()->add("nf1", STRING("ABCD"))
              ->add("nf2", STRUCT()->add("nnf1", MAP(STRING("AB"), STRING("HKMK")))));
  schema.add_field("field2",
      ARRAY("STRUCT")->add(STRUCT()->add("nf2", STRING("FEWDSF"))
                                   ->add("nf3", STRING("ABCDEFS")))
                     ->add(STRUCT()->add("nf2", STRING("BJIOJW"))
                                   ->add("nf3", STRING("JIOJIJOIJO")))
                     ->add(STRUCT()->add("nf2", STRING("DDDDDD"))
                                   ->add("nf3", STRING("KOUIYTUOIJIIO"))));
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
  schema.add_field("height(cm)", F64("178.4"));  
  schema.add_field("weight(KG)", I32("65")); 
  schema.add_field("salay", STRING("0"));
  schema.add_field("address", STRUCT()->add("street", STRING("HongJae Chungu 3-cha Apartment"))
                                      ->add("city", STRING("SEOUL"))
                                      ->add("country", STRING("123")));
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

TEST(VARIANT_TEST, HANDLES_VARIANT_1) {
  
  Schema schema("VariantHandle");
  Array* arr = ARRAY("STRUCT");
  // For 5000 array elements, we need element length metadata(5000)
  // that 1 byte is not sufficient to store it.
  // For handling this, all length metadata is recorded 
  // supporting variant length.
  // This test's purpose is to test whether our encoding/decoding
  // supports variant types. 
  for(int i = 0; i < 5000; i++) {
    arr->add(STRING("array"));
  }
  schema.add_field("field2", arr);
  Encoder e1,e2;
  Decoder d;
  Bytes bytes1 = e1.encode(schema);
  Bytes bytes2 = e2.encode(d.decode(bytes1));
  EXPECT_EQ(bytes1, bytes2);
}