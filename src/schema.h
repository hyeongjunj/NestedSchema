#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <bitset>
#include <queue>
#include <iostream>

typedef std::vector<std::byte> Bytes;
/*
enum DATATYPE
{
  INT8,
  INT16,
  INT32,
  INT64,
  FLOAT32,
  FLOAT64,
  STRING,
  ARRAY,
  STRUCT,
  MAP
};
*/

class Data {
private:
  std::string data_type_;
  std::string value_;
  bool isPrimitive_;
  int type_size_;
public:
  Data(){isPrimitive_ = false;} 
  ~Data(){}
  int Size() {return type_size_;}
  void SetType(std::string data_type, bool tf) {
    data_type_ = data_type;
    isPrimitive_ = tf;
  }
  void SetValue(std::string value) {value_ = value;}
  bool isPrimitive() {return isPrimitive_;}
  std::string Type() {return data_type_;}
  std::string Value() {return value_;}
  virtual int numofElements() {return 0;}
  virtual const std::vector<Data*>& Get() {
    std::cout<<"[FATAL ERROR] : Invalid Access!\n"; 
    return {};} 
  // TODO : FIX THIS! This Get() will not be used in Primitive class
  // So we need some nicer design for this..
};

class Primitive : public Data {
public:
  Primitive(std::string data_type,
                std::string value) 
                {
                  SetType(data_type, true);
                  SetValue(value);
                }
};

class Array : public Data {
private:
  std::vector<Data*> array_;
  std::string data_type_;
public:
  Array() {
    SetType("ARRAY", false);
  }
  Array(Data* data) {
    SetType("ARRAY", false);
    data_type_.assign(data->Type(), sizeof(data->Type()));
  }
  void Add2Array(Data* data) {
    array_.push_back(data);
  }
  virtual int numofElements() override {
    return array_.size();
  }
  virtual const std::vector<Data*>& Get() override 
  {return array_;}
};

class Struct : public Data {
private:
  std::vector<Data*> struct_;
  std::vector<std::string> field_name_;
  //std::vector<int> offset_;
public:
  Struct() {
    SetType("STRUCT", false);
  }
  void Add2Struct(std::string field, Data* data) {
    struct_.push_back(data);
    field_name_.push_back(field);
  }
  virtual int numofElements() override {
    return struct_.size();
  }
  virtual const std::vector<Data*>& Get() override 
  {return struct_;}
};

class Map : public Data {
private:
  std::vector<Data*> map_;
  //int offset_[2];
public:
  Map(Data* key, Data* value) {
    map_.push_back(key);
    map_.push_back(value);
    SetType("MAP", false);
  }
  virtual const std::vector<Data*>& Get() override 
  {return map_;}
  virtual int numofElements() override {
    return 2;
  }
};

class Schema {
private:
  std::unordered_map<std::string, Data*> SchemaElements_;
  /*
  store the list in form of <field name, value>
  supported datatypes are :
  MAP, STRUCT, LIST and PRIMITIVES
  */
public:
  ~Schema() {}
  //void GetFieldInput(std::string input);
  void AddElement(std::string field_name, Data* data) {
    SchemaElements_.insert({field_name, data});
  }
  const std::unordered_map<std::string, Data*>& Get() {
    return SchemaElements_;
  }
  Data* STRING(std::string str) {
    return new Primitive("STRING", str);
  }
  Data* MAP(Data* key, Data* value) {
    return new Map(key, value);
  }
  Data* ARRAY(Data* data) {
    return new Array();
  }
};