#include "encoding.h"
#include "gtest/gtest.h"
#include <stack>
using namespace nestedSchema;
/*
This TEST tests whether nested schema can be stored properly in this
data structure.
After the input of schema, we traverse schema, check each of field is 
stored correctly or not.
*/
/*
  NestedExample  
  field1: Struct {nf1: String, nf2: Struct { nnf1: Map<String, String> }} 
  field2: Array<Struct {nf2: String, nf3: String }
*/
std::vector<std::string> SchemaTraverse(Schema& schema) {
  std::vector<std::string> ret;
  Data* root = schema.Get();
  std::stack<Data*> Stack;
  Stack.push(root);
  while(!Stack.empty()) {
    Data* topedElement = Stack.top();
    if(topedElement->isPrimitive()) {
      ret.push_back(topedElement->Value());
      Stack.pop();
    } else {
      Stack.pop();
      for(int i = topedElement->Get().size() -1; i >= 0; i--) {
        Stack.push(topedElement->Get()[i]);       
      }
    }
  }
  return ret;
}

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
  std::vector<std::string> elements = 
    {"NestedExample", "ABCD", "AB", "HKMK", "FEWDSF", "ABCDEFS", "BJIOJW", 
     "JIOJIJOIJO", "DDDDDD", "KOUIYTUOIJIIO"};
     
  EXPECT_EQ(SchemaTraverse(schema), elements);
  
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
  schema.add_field("first name", STRING("HYEONGJUN")); 
  schema.add_field("last name", STRING("JEON"));
  schema.add_field("height(cm)", F64("178.87"));  
  schema.add_field("weight(KG)", F32("65.3")); 
  schema.add_field("salay", I32("0"));
  schema.add_field("address", STRUCT()->add("street", STRING("HongJae Chungu 3-cha Apartment"))
                                      ->add("city", STRING("SEOUL"))
                                      ->add("country", STRING("KOREA")));
  schema.add_field("properties", MAP(STRING("IHAVE"),STRING("NOTHING")));
  schema.add_field("assets", ARRAY("STRING")->add(STRING("HOUSE"))
                                            ->add(STRING("CAR"))
                                            ->add(STRING("PRIVATE JET")));

  std::vector<std::string> elements = 
    {"Person", "HYEONGJUN", "JEON", "178.87", "65.3", "0",
     "HongJae Chungu 3-cha Apartment", "SEOUL", "KOREA", "IHAVE", "NOTHING", 
     "HOUSE", "CAR", "PRIVATE JET"};

  EXPECT_EQ(SchemaTraverse(schema), elements);
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

  std::vector<std::string> elements = 
    {"TEST3", "12345678", "FED", "CKOE", "DSAD", "DSAV", "DSADS",
     "BBB", "CDA", "DSADASFEQG"};

  EXPECT_EQ(SchemaTraverse(schema), elements);
}  