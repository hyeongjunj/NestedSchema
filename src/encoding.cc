#include "encoding.h"
#include <string>
#include <cstdio>

// for primitive types
Bytes Encoder::VarientEncoding(int64_t value) {
	Bytes encodedBytes;
  if(value > 127) std::cout<<"ERROR!\n";
  std::byte b{value};
  encodedBytes.push_back(b);
  char* charBytes = reinterpret_cast<char*>(encodedBytes.data());
  
	return encodedBytes;
}

void Encoder::StringEncoding(Data* data) {
  // for string type, type + length + string bytecode
  // length should be encodeded with VarientEncoding
  Bytes bstream;
  
  Bytes type = TypeEncoding(data);
  Bytes length = VarientEncoding(data->Value().size());
  
  for(std::byte b : type) bstream.push_back(b);
  for(std::byte b : length) bstream.push_back(b);

  const char* str = data->Value().c_str();
  for(int i = 0; i < data->Value().size(); i++) {
    bstream.push_back(std::byte(str[i]));
  }
  encodeStream_.push_back({bstream, data});
  return;
}

Bytes Encoder::TypeEncoding(Data* data) {
  std::string type = data->Type();
  int64_t encodeType;
  if(type.compare("I8") == 0)          encodeType = 0;
  else if(type.compare("I16") == 0)    encodeType = 1;
  else if(type.compare("I32") == 0)    encodeType = 2;
  else if(type.compare("I64") == 0)    encodeType = 3;
  else if(type.compare("F32") == 0)    encodeType = 4;
  else if(type.compare("F64") == 0)    encodeType = 5;
  else if(type.compare("STRING") == 0) encodeType = 6;
  else if(type.compare("STRUCT") == 0) encodeType = 7;
  else if(type.compare("MAP") == 0)    encodeType = 8;
  else if(type.compare("ARRAY") == 0)  encodeType = 9;
  return VarientEncoding(encodeType);
}

void Encoder::PrepareEncodingMeta() {
  encodeStream_.emplace_back();
  // This will later be filled by EncodingMeta()
}

void Encoder::EncodingMeta(Data* data, int meta_pos) {
  Bytes metadata;
  int metadata_size = 1;
  std::string data_type = data->Type();
  int bsize = 0;
  int nestedsize = 0;

  Bytes type = TypeEncoding(data);
  for(std::byte b : type) metadata.push_back(b);

  if(data_type.compare("ARRAY") == 0) {
    // Metadata format : [TYPE][NUMBER OF ELEMENTS][EACH ELEMENT LENGTH]...
    int fieldNum = data->numofElements();
    Bytes fieldNumMeta = VarientEncoding(fieldNum);
    for(std::byte b : fieldNumMeta) metadata.push_back(b);
    metadata_size += fieldNumMeta.size();
    Data* DataTraverse = data->Get()[0];
    int fieldIdx = 0;
    for(int i = meta_pos + 1; i < encodeStream_.size(); i++) {
      if(encodeStream_[i].second->Type().compare(DataTraverse->Type()) == 0) {
        if(!DataTraverse->isPrimitive()) {
          bsize = meta_size_offset[i];
          i = meta_offset[i];
        }
        else {
          bsize = encodeStream_[i].first.size();
        } 
        nestedsize += bsize;
        Bytes fieldSize = VarientEncoding(bsize);
        metadata_size += (int)fieldSize.size();

        Bytes length = VarientEncoding(bsize);
        for(std::byte b : length) metadata.push_back(b);
        if(DataTraverse == data->Get()[fieldNum - 1]) {
          meta_offset[meta_pos] = i; // endpoint of this type 
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          encodeStream_[meta_pos].first = metadata;
          encodeStream_[meta_pos].second = data;
          break;
        }
        DataTraverse = data->Get()[++fieldIdx];
      }
    }
  } 
  else if(data_type.compare("MAP") == 0) {
  // Metadata format : [TYPE][KEY LOCATION][VALUE LOCATION]
    Data* KeyType   = data->Get()[0]; // Key
    Data* ValueType = data->Get()[1]; // Value
    Data* DataTraverse = KeyType;
    for(int i = meta_pos + 1; i < encodeStream_.size(); i++) {
      if(encodeStream_[i].second->Type().compare(DataTraverse->Type()) == 0) {
        if(!DataTraverse->isPrimitive()) {
          bsize = meta_size_offset[i];
          i = meta_offset[i];
        }
        else {
          bsize = encodeStream_[i].first.size();
        } 
        nestedsize += bsize;
        Bytes fieldSize = VarientEncoding(bsize);
        metadata_size += (int)fieldSize.size();

        Bytes length = VarientEncoding(bsize);
        for(std::byte b : length) metadata.push_back(b);
        if(DataTraverse == ValueType) {
          meta_offset[meta_pos] = i; // endpoint of this type
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          encodeStream_[meta_pos].first = metadata;
          encodeStream_[meta_pos].second = data;
          break;
        }
        DataTraverse = ValueType;
      }
    }
  }
  else if(data_type.compare("STRUCT") == 0) {
    // Metadata foramt : [TYPE][NUMBER OF FILEDS]
    //                   [FIELD NAME]...[ELEMENT LENGTH]...
    // Array and Struct's metadata format is similiar to each other
    // Encode field number
    int fieldNum = data->numofElements();
    Bytes fieldNumMeta = VarientEncoding(fieldNum);
    for(std::byte b : fieldNumMeta) metadata.push_back(b);
    metadata_size += fieldNumMeta.size();
    // encode field names
    for(int i = 0; i < data->numofElements(); i++) {
      Bytes bstream;
      std::string fieldName = ((Struct*)data)->fieldName(i);
      const char* str = fieldName.c_str();
      Bytes length = VarientEncoding(fieldName.size());
      for(std::byte b : length) metadata.push_back(b);
      for(int i = 0; i < fieldName.size(); i++) {
        metadata.push_back(std::byte(str[i]));
      }
      metadata_size += length.size() + fieldName.size();
    }
    // encode each element's lengths 
    Data* DataTraverse = data->Get()[0];
    int fieldIdx = 0;
    for(int i = meta_pos + 1; i < encodeStream_.size(); i++) {
      if(encodeStream_[i].second->Type().compare(DataTraverse->Type()) == 0) {
        if(!DataTraverse->isPrimitive()) {
          bsize = meta_size_offset[i];
          i = meta_offset[i];
        }
        else {
          bsize = encodeStream_[i].first.size();
        } 
        nestedsize += bsize;
        Bytes fieldSize = VarientEncoding(bsize);
        metadata_size += (int)fieldSize.size();

        Bytes length = VarientEncoding(bsize);
        for(std::byte b : length) metadata.push_back(b);
        
        if(DataTraverse == data->Get()[fieldNum - 1]) {
          meta_offset[meta_pos] = i; // endpoint of this type
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          encodeStream_[meta_pos].first = metadata;
          encodeStream_[meta_pos].second = data;
          break;
        }
        DataTraverse = data->Get()[++fieldIdx];
      }
    }
  }
  else {
    std::cerr<<"[FATAL ERROR] Wrong Data Type\n";
  }
  Bytes b;
  return;
}

void Encoder::EncodePrimitiveType(Data* data) {
  if(data->Type().compare("STRING") == 0) {
    StringEncoding(data);
  }
  else {
    Bytes type = TypeEncoding(data);
    Bytes value = VarientEncoding(std::stoi(data->Value()));
    Bytes bstream;
    for(std::byte b : type) bstream.push_back(b);
    for(std::byte b : value) bstream.push_back(b);
    encodeStream_.push_back({bstream, data});
  }
  return;
}

/*
For encoding, we first traverse the schema.
Each schema element type is categorized by two types :
primitive type, and non-primitive type.
We do not need metadata for primitive type except
type and length for string, but
we need metadata for non-primitive type.
(Struct, Map, Array)
<Encoding procedure during schema traverse>
1. Each primitive schema element is first encoded in Bytes vector.
2. non-primitive type element's metadata(e.g., field length)
cannot be determined at the point, so we allocate empty bytes.
3. Stack up the non-primitive types for later metadata encoding
4. After finish, pop each non-primitive types for metadata encoding
5. sums up!

*/
Bytes Encoder::encode(Schema& schema) {
  // we have to traverse the schema for encoding 
  std::stack<Data*> TraverseStack;
  std::stack<std::pair<Data*, int>> MetaStack;

  TraverseStack.push(schema.Get());
  int encodingIdx = 0;

  while(!TraverseStack.empty()) {
    if(TraverseStack.top()->isPrimitive()) {
      EncodePrimitiveType(TraverseStack.top());
      TraverseStack.pop();
    } else {
      meta_offset[encodingIdx] = -1;
      // Since we have no idea how long or how many elements 
      // in here, we just prepare encoding by push to the stack 
      // and emplace_back().
      PrepareEncodingMeta();
      MetaStack.push({TraverseStack.top(), encodingIdx}); 
          
      Data* popedElement = TraverseStack.top();
      TraverseStack.pop();
      // Traverse is done similiar to DFS way.
      for(int i = popedElement->Get().size() -1; i >= 0; i--) {
        TraverseStack.push(popedElement->Get()[i]);       
      }
    }
    encodingIdx++;
  }
  // Traverse finishes, so we can now encode metadata.
  for(int i = meta_offset.size()-1; i >= 0 ; i--) {
    EncodingMeta(MetaStack.top().first, MetaStack.top().second);
    MetaStack.pop();
  }
  Bytes bytestream;
  // Sums up and return full bytestream.
  for(std::pair<Bytes, Data*> bytes : encodeStream_) {
    for(std::byte b : bytes.first) {
      bytestream.push_back(b);
    }
  }
  return bytestream;
}

/*
Decoding will be done in recursive way.
We have the information in metadata of non-primitive type about
field number and length of fields, etc...
By using that, we can call decoding function recursively.
*/
Schema& Decoder::decode(Bytes &bytes) {
  char* charBytes = reinterpret_cast<char*>(bytes.data());
  int i = 0;
  Struct* data = (Struct*)PartialDecode(charBytes, bytes.size() - 1);
  schema.AddElement("schema", data);
  return schema;
}

// first index indicates the starting point
Data* Decoder::PartialDecode(char* charBytes, int endIdx) {
  Data* data;
  // decodingIdx indicates the starting byte.
  // We have to recursively decode 
  // from [decodingIdx] to [decodingIdx + fieldSize - 1]
  int decodingIdx = 0;
  char type = charBytes[decodingIdx];  // 1 byte for TYPE decoding
  decodingIdx += TYPEMETA;
  int variantLength = 1;
  int numofFields;
  if(type == 7) { // struct
    Struct* str = new Struct();
    numofFields = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    std::vector<std::string> fieldName(numofFields);
    for(int i = 0; i < numofFields; i++) {
      int length = VariantDecoder(charBytes + decodingIdx, decodingIdx);
      fieldName[i] = std::string(charBytes + decodingIdx, length);
      decodingIdx += length;
    }
    std::vector<int64_t> fieldSize(numofFields);
    for(int i = 0; i < numofFields; i++) {
      fieldSize[i] = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    }
    for(int i = 0; i < numofFields; i++) {
      str->Add2Struct(fieldName[i], PartialDecode(charBytes + decodingIdx, 
                               decodingIdx + fieldSize[i] -1));
      decodingIdx += fieldSize[i];
    }
    data = str;
  }
  else if(type == 8) { // map
    int64_t keyLength   = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    int64_t valueLength = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    Data* Key   = PartialDecode(charBytes + decodingIdx, decodingIdx + keyLength   -1);
    decodingIdx += keyLength;
    Data* Value = PartialDecode(charBytes + decodingIdx, decodingIdx + valueLength -1);
    data = new Map(Key, Value);
  }
  else if(type == 9) { // array
    Array* arr = new Array();
    numofFields = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    std::vector<int64_t> fieldSize(numofFields);
    for(int i = 0; i < numofFields; i++) {
      fieldSize[i] = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    }
    for(int i = 0; i < numofFields; i++) {
      arr->Add2Array(PartialDecode(charBytes + decodingIdx, decodingIdx + fieldSize[i] -1));
      decodingIdx += fieldSize[i];
    }
    data = arr;
  }
  else {
    data = PrimitiveTypeDecode(charBytes);
  }
  return data;
}

int Decoder::VariantDecoder(char* start_point, int& offset) {
  offset++; 
  for(int i = 0; ; i++) {
    if(start_point[i] & 128 == 128) { // TODO : Change this. This logic is not working
      //std::cout<<"Variant Increase\n";
      //offset++;
    }
    else break;
  }
  return (int)start_point[0]; // TODO : have to be changed
}

Data* Decoder::PrimitiveTypeDecode(char* charBytes) {
  Data* data;
  int decodingIdx = 0;
  char type = (char)charBytes[decodingIdx];
  decodingIdx++;
  int reserve = decodingIdx; 
  int64_t length = VariantDecoder(charBytes + decodingIdx, decodingIdx);
  /*
  VariantDecoder changes decodingIdx but we do not need that 
  when decoding number. So, if this is not type string, 
  going back to previous position.
  */
  if(type == 0) 
    data = new Primitive("I8",     std::to_string(length));
  else if(type == 1)
    data = new Primitive("I16",    std::to_string(length));
  else if(type == 2) 
    data = new Primitive("I32",    std::to_string(length));
  else if(type == 3)  
    data = new Primitive("I64",    std::to_string(length));
  else if(type == 4) 
    data = new Primitive("F32",    std::to_string(length));
  else if(type == 5) 
    data = new Primitive("F64",    std::to_string(length));
  else if(type == 6) 
    data = new Primitive("STRING", std::string(charBytes + decodingIdx, length));
  else {
    std::cerr<<"ERROR : Failed to Decode\n";
  }
  return data;
}