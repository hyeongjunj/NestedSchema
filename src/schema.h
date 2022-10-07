#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <bitset>
#include <queue>
#include <iostream>
typedef std::vector<std::byte> Bytes;

class Data {
private:
  std::string data_type_;
  std::string value_;
  bool isPrimitive_;
  int type_size_;
public:
  Data(){isPrimitive_ = false;} 
  virtual ~Data(){}
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
    std::cout<<"[ERROR] : Invalid Access\n"; 
    return {};} 
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
  std::string element_type_;
public:
  Array() {
    SetType("ARRAY", false);
  }
  Array(std::string element_type) {
    SetType("ARRAY", false);
    element_type_.assign(element_type, element_type.size());
  }
  Array(Data* data) {
    SetType("ARRAY", false);
    data_type_.assign(data->Type(), sizeof(data->Type()));
  }
  void Add2Array(Data* data) {
    array_.push_back(data);
  }
  Array* add(Data* data) {
    array_.push_back(data);
    return this;
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
public:
  Struct() {
    SetType("STRUCT", false);
  }
  void Add2Struct(std::string field, Data* data) {
    struct_.push_back(data);
    field_name_.push_back(field);
  }
  Struct* add(std::string field, Data* data) {
    struct_.push_back(data);
    field_name_.push_back(field);
    return this;
  }
  std::string fieldName(int idx) {
    if(idx < field_name_.size())
      return field_name_[idx];
    else {
      std::cerr<<"[ERROR] Wrong access\n";
      return "";
    }
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
  Struct* struct_;
  Data* schema_;
  std::string name_;
public:
  Schema() {}
  Schema(std::string name) {
    struct_ = new Struct(); 
    struct_->add("Schema Name", new Primitive("STRING", name));
    name_ = name;
    schema_ = struct_;
    
  }
  ~Schema() {}
  void AddElement(std::string field_name, Data* data) {
    schema_ = data;
  }
  Data* Get() {
    return schema_;
  }
  void add_field(std::string field_name, Data* data) {
    struct_->add(field_name, data);
  }
};

namespace nestedSchema {
  inline Primitive* STRING(std::string str) {
    return new Primitive("STRING", str);
  }
  inline Map* MAP(Data* key, Data* value) {
    return new Map(key, value);
  }
  inline Array* ARRAY(std::string type) {
    return new Array(type);
  }
  inline Struct* STRUCT() {
    return new Struct();
  }  
  inline Primitive* I8(std::string str) {
    return new Primitive("I8", str);
  }
  inline Primitive* I16(std::string str) {
    return new Primitive("I16", str);
  }
  inline Primitive* I32(std::string str) {
    return new Primitive("I32", str);
  }
  inline Primitive* I64(std::string str) {
    return new Primitive("I64", str);
  }
  inline Primitive* F32(std::string str) {
    return new Primitive("F32", str);
  }
  inline Primitive* F64(std::string str) {
    return new Primitive("F64", str);
  }
} 