#include "encoding.h"
#include <string>
#include <cstdio>

// for primitive types
Bytes Encoder::VarientEncoding(int64_t value) {
	Bytes encodedBytes;
  if(value > 127) std::cout<<"ERROR!\n";
  std::byte b{value};
  encodedBytes.push_back(b);
  /*
  std::byte Header;
  int bytes = 8; // how many bytes we need at maximum
                 // TODO : it can vary with data_types.
  int64_t value = std::stoi(data->Value());
  int64_t tmp = 1;
  int64_t MSB = 1;
  tmp << 8;
  tmp--; // 7 bits
  MSB <<= 8;
  int idx = 0;
  while(value | tmp) {
    int64_t value4store = value | tmp;
    value4store >>= 7*idx;
    if(idx > 0) {
      value4store |= MSB;  
    }
    std::byte b{(char)value4store};
    encodedBytes.push_back(b);
    idx++;
    //MSB <<= 8; 
    tmp <<= 8;
  }
  */
  //encodeStream_.push_back({encodedBytes, data});
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
  std::cout<<"       --> "<<bstream.size()<<"\n";
  encodeStream_.push_back({bstream, data});
  return;
}

Bytes Encoder::TypeEncoding(Data* data) {
  // i8      -> 0, 
  //i16      -> 1
  //i32      -> 2
  //i64      -> 3
  //f32      -> 4
  //f64      -> 5
  //String   -> 6
  //Struct   -> 7
  //Map      -> 8
  //Array    -> 9
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
  std::cout<<"Encoding Meta\n";
  Bytes metadata;
  int metadata_size = 1;
  std::string data_type = data->Type();
  int bsize = 0;
  int nestedsize = 0;

  Bytes type = TypeEncoding(data);
  for(std::byte b : type) metadata.push_back(b);

  if(data_type.compare("ARRAY") == 0) {
    // Metadata format : [TYPE][NUMBER OF ELEMENTS][LAST LOCATION]
    //std::cout<<"ARRAY META     "<<meta_pos<<"\n";
    int fieldNum = data->numofElements();
    Bytes fieldNumMeta = VarientEncoding(fieldNum);
    for(std::byte b : fieldNumMeta) metadata.push_back(b);
    //encodeStream_.push_back({fieldNumMeta, data});
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
        std::cout<<"                                 FieldIdx "<<i<<" "<<bsize<<"\n"; ///////////////
        Bytes fieldSize = VarientEncoding(bsize);
        metadata_size += (int)fieldSize.size();

        Bytes length = VarientEncoding(bsize);
        for(std::byte b : length) metadata.push_back(b);
        //encodeStream_.insert(encodeStream_.begin() + meta_pos, 
        //                    std::make_pair(metadata, data));
        if(DataTraverse == data->Get()[fieldNum - 1]) {
          meta_offset[meta_pos] = i; // endpoint of this type 
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          std::cout<<"ARRAY META     "<<"["<<meta_pos<<"]"<<"["<<i<<"]"<<"["<<nestedsize<<"]"<<"\n";
          std::cout<<"metadata size : "<<meta_size_offset[meta_pos]<<" bytes\n";
          std::cout<<"metadata size : "<<metadata_size<<" bytes\n";
          std::cout<<"metadata size : "<<nestedsize<<" bytes\n";
          //encodeStream_.insert(encodeStream_.begin() + meta_pos, {metadata, data});
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
        std::cout<<"                                 FieldIdx "<<i<<" "<<bsize<<"\n"; //////////////
        Bytes fieldSize = VarientEncoding(bsize);
        metadata_size += (int)fieldSize.size();

        Bytes length = VarientEncoding(bsize);
        for(std::byte b : length) metadata.push_back(b);
        //encodeStream_.insert(encodeStream_.begin() + meta_pos, 
        //                     std::make_pair(metadata, data));
        if(DataTraverse == ValueType) {
          meta_offset[meta_pos] = i; // endpoint of this type
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          std::cout<<"MAP META       "<<"["<<meta_pos<<"]"<<"["<<i<<"]"<<"\n";
          std::cout<<"metadata size : "<<metadata.size()<<" bytes\n";
          //encodeStream_.insert(encodeStream_.begin() + meta_pos, {metadata, data});
          encodeStream_[meta_pos].first = metadata;
          encodeStream_[meta_pos].second = data;
          break;
        }
        DataTraverse = ValueType;
      }
    }
  }
  else if(data_type.compare("STRUCT") == 0) {
    // Metadata foramt : [TYPE][NUMBER OF FILEDS][LOCATION(BYTES)][LOCTION]...
    int fieldNum = data->numofElements();
    Bytes fieldNumMeta = VarientEncoding(fieldNum);
    for(std::byte b : fieldNumMeta) metadata.push_back(b);
    //encodeStream_.push_back({fieldNumMeta, data});
    metadata_size += fieldNumMeta.size();
    Data* DataTraverse = data->Get()[0];
    int fieldIdx = 0;
    std::cout<<"fieldnum : "<<fieldNum<<"\n";
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
        std::cout<<"                                 FieldIdx "<<i<<" "<<bsize<<"\n"; //////////////
        Bytes fieldSize = VarientEncoding(bsize);
        metadata_size += (int)fieldSize.size();

        Bytes length = VarientEncoding(bsize);
        for(std::byte b : length) metadata.push_back(b);
        
        if(DataTraverse == data->Get()[fieldNum - 1]) {
          meta_offset[meta_pos] = i; // endpoint of this type
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          std::cout<<"STRUCT META    "<<"["<<meta_pos<<"]"<<"["<<i<<"]"<<"\n";
          std::cout<<"metadata size : "<<metadata.size()<<" bytes\n";   
          //encodeStream_.insert(encodeStream_.begin() + meta_pos, {metadata, data});
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
  //encodeStream_[meta_pos] = {b, data};
  return;
}

void Encoder::EncodePrimitiveType(Data* data) {
  if(data->Type().compare("STRING") == 0) {
    StringEncoding(data);
  }
  else {
    std::cout<<data->Value()<<"\n";
    VarientEncoding(std::stoi(data->Value()));
  }
  return;
}

Bytes Encoder::encode(Schema& schema) {
  // we have to traverse the schema for encoding 
  for(const auto& fields : schema.Get()) {
    // fields.first is field name,
    // fields.second is value(Data type)
    if(fields.second->isPrimitive()) { // end of the traversal tree
      std::cout<<fields.second->Value()<<"\n";
    } else { // we have to traverse deep into this data type
             // struct, map, list type
      std::stack<Data*> TraverseStack;
      std::stack<std::pair<Data*, int>> MetaStack;
      //std::stack<std::pair<Data*, int>> EncodingMetaStack;
      TraverseStack.push(fields.second);
      int encodingIdx = 0;

      while(!TraverseStack.empty()) {
        if(TraverseStack.top()->isPrimitive()) {
          std::cout<<TraverseStack.top()->Value()<<" "<<encodingIdx<<"\n";
          EncodePrimitiveType(TraverseStack.top());
          TraverseStack.pop();
        } else {
          meta_offset[encodingIdx] = -1;
          
          std::cout<<"Type : "<<TraverseStack.top()->Type()<<" "<<encodingIdx<<"\n";
          PrepareEncodingMeta();
          MetaStack.push({TraverseStack.top(), encodingIdx}); 
          
          Data* popedElement = TraverseStack.top();
          TraverseStack.pop();
          
          for(int i = popedElement->Get().size() -1; i >= 0; i--) {
            TraverseStack.push(popedElement->Get()[i]);       
          }
        }
        encodingIdx++;
      }
      for(int i = meta_offset.size()-1; i >= 0 ; i--) {
        //std::cout<<"--------> "<<MetaStack.top().first->Type()<<"\n";
        EncodingMeta(MetaStack.top().first, MetaStack.top().second);
        MetaStack.pop();
      }
    }
  }
  Bytes bytestream;
  for(std::pair<Bytes, Data*> bytes : encodeStream_) {
    std::cout<<bytes.first.size()<<" bytes..\n";
    for(std::byte b : bytes.first) {
      bytestream.push_back(b);
    }
  }
  return bytestream;
}

Schema& Decoder::decode(Bytes &bytes) {
  char* charBytes = reinterpret_cast<char*>(bytes.data());
  int i = 0;
  Struct* data = (Struct*)PartialDecode(charBytes, bytes.size() - 1);
  schema.AddElement("schema", data);
  return schema;
}

//return number
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

Data* Decoder::PartialDecode(char* charBytes, int endIdx) {
  Data* data;
  int decodingIdx = 0;
  char type = charBytes[decodingIdx];  // 1 byte for TYPE decoding
  decodingIdx += TYPEMETA;
  int variantLength = 1;
  int numofFields;
  if(type == 7) { // struct
    std::cout<<" ******* Struct Decoding *******\n";
    Struct* str = new Struct();
    numofFields = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    std::vector<int64_t> fieldSize(numofFields);
    for(int i = 0; i < numofFields; i++) {
      fieldSize[i] = VariantDecoder(charBytes + decodingIdx, decodingIdx);
      std::cout<<fieldSize[i]<<"\n";
    }
    // up to this point was metadata decoding
    for(int i = 0; i < numofFields; i++) {
      // decodingIdx is the staring bytes.
      // We have to recursively decode 
      // from [decodingIdx] to [decodingIdx + fieldSize - 1]
      str->Add2Struct("field", PartialDecode(charBytes + decodingIdx, 
                               decodingIdx + fieldSize[i] -1));
      decodingIdx += fieldSize[i];
    }
    data = str;
  }
  else if(type == 8) { // map
    std::cout<<" ******* Map Decoding *******\n";
    int64_t keyLength   = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    int64_t valueLength = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    Data* Key   = PartialDecode(charBytes + decodingIdx, decodingIdx + keyLength   -1);
    std::cout<<"K "<<decodingIdx<<" V "<<decodingIdx + keyLength   -1<<"\n";
    decodingIdx += keyLength;
    Data* Value = PartialDecode(charBytes + decodingIdx, decodingIdx + valueLength -1);
    std::cout<<"K "<<decodingIdx<<" V "<<decodingIdx + keyLength   -1<<"\n";
    data = new Map(Key, Value);
  }
  else if(type == 9) { // array
    Array* arr = new Array();
    std::cout<<" ******* Array Decoding *******\n";
    numofFields = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    std::vector<int64_t> fieldSize(numofFields);
    for(int i = 0; i < numofFields; i++) {
      fieldSize[i] = VariantDecoder(charBytes + decodingIdx, decodingIdx);
      std::cout<<fieldSize[i]<<"\n";
    }
    // up to this point was metadata decoding
    for(int i = 0; i < numofFields; i++) {
      // decodingIdx is the staring bytes.
      // We have to recursively decode 
      // from [decodingIdx] to [decodingIdx + fieldSize - 1]
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

Data* Decoder::PrimitiveTypeDecode(char* charBytes) {
  Data* data;
  int decodingIdx = 0;
  char type = (char)charBytes[decodingIdx];
  decodingIdx++;
  int64_t length = VariantDecoder(charBytes + decodingIdx, decodingIdx);
  if(type == 0) 
    data = new Primitive("I8",     std::string(charBytes + decodingIdx, length));
  else if(type == 1)
    data = new Primitive("I16",    std::string(charBytes + decodingIdx, length));
  else if(type == 2) 
    data = new Primitive("I32",    std::string(charBytes + decodingIdx, length));
  else if(type == 3)  
    data = new Primitive("I64",    std::string(charBytes + decodingIdx, length));
  else if(type == 4) 
    data = new Primitive("F32",    std::string(charBytes + decodingIdx, length));
  else if(type == 5) 
    data = new Primitive("F64",    std::string(charBytes + decodingIdx, length));
  else if(type == 6) {
    data = new Primitive("STRING", std::string(charBytes + decodingIdx, length));
    std::cout<<"                                                        "<<data->Value()<<"\n";
  }
  else {
    std::cerr<<"FATAL ERROR : Failed to Decode\n";
  }
  return data;
}