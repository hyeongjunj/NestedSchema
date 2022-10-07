#include "encoding.h"
#include <string>
#include <cstdio>
#include <bitset>
#include <cmath>

// for primitive types & metadata(e.g., length of element) 
// element length should be encodeded with VarientEncoding
// because the length size(bytes) is not fixed to certain bytes.
// (E.g., one byte is enough for storing "5", but not for "1000")
Bytes Encoder::VarientEncoding(int64_t value) {
	unsigned char MSB = 1;
  MSB <<= 7;
	std::vector<std::byte> encodedBytes;
	unsigned char byte_;
	std::vector<unsigned char> bytes_;
	int idx = 0;
  while(value != 0) {
		byte_ = value & (MSB -1);
		bytes_.push_back(byte_);
		value >>= 7;
		idx++;
	}
	for(int i = bytes_.size() -1; i >= 0; i--) {
		if(i != 0) {
			bytes_[i] = bytes_[i] | MSB;
		}
		std::byte b{bytes_[i]};
		encodedBytes.push_back(b);
	}
	return encodedBytes;
}

void Encoder::StringEncoding(Data* data) {
  // for string type, type + length + string bytecode
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
    std::cerr<<"[ERROR] Wrong Data Type\n";
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

