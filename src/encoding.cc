#include "encoding.h"
#include <string>

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
  // for string type, length + string bytecode
  // length should be encodeded with VarientEncoding
  Bytes bstream;
  bstream.push_back(std::byte{67}); // TODO: Change. Temporal usage..
  const char* str = data->Value().c_str();
  for(int i = 0; i < data->Value().size(); i++) {
    bstream.push_back(std::byte(str[i]));
  }
  encodeStream_.push_back({bstream, data});
  return;
}

void Encoder::PrepareEncodingMeta() {
  encodeStream_.emplace_back();
  // This will later be filled by EncodingMeta()
}

void Encoder::EncodingMeta(Data* data, int meta_pos) {
  Bytes metadata;
  int metadata_size = 0;
  std::string data_type = data->Type();
  int bsize = 0;
  int nestedsize = 0;
  if(data_type.compare("ARRAY") == 0) {
    // Metadata format : [TYPE][NUMBER OF ELEMENTS][LAST LOCATION]
    //std::cout<<"ARRAY META     "<<meta_pos<<"\n";
    int fieldNum = data->numofElements();
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
        encodeStream_.push_back({VarientEncoding(fieldSize.size()), data});
        if(DataTraverse == data->Get()[fieldNum - 1]) {
          meta_offset[meta_pos] = i; // endpoint of this type 
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          std::cout<<"ARRAY META     "<<"["<<meta_pos<<"]"<<"["<<i<<"]"<<"["<<nestedsize<<"]"<<"\n";
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
        encodeStream_.push_back({VarientEncoding(fieldSize.size()), data});
        if(DataTraverse == ValueType) {
          meta_offset[meta_pos] = i; // endpoint of this type
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          std::cout<<"MAP META       "<<"["<<meta_pos<<"]"<<"["<<i<<"]"<<"\n";
          break;
        }
        DataTraverse = ValueType;
      }
    }
  }
  else if(data_type.compare("STRUCT") == 0) {
    // Metadata foramt : [TYPE][NUMBER OF FILEDS][LOCATION(BYTES)][LOCTION]...
    int fieldNum = data->numofElements();
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
        encodeStream_.push_back({VarientEncoding(fieldSize.size()), data});
        if(DataTraverse == data->Get()[fieldNum - 1]) {
          meta_offset[meta_pos] = i; // endpoint of this type
          meta_size_offset[meta_pos] = nestedsize + metadata_size;
          std::cout<<"STRUCT META    "<<"["<<meta_pos<<"]"<<"["<<i<<"]"<<"\n";
          break;
        }
        //std::cout<<"                                 FieldIdx "<<i<<"\n";
        DataTraverse = data->Get()[++fieldIdx];
        
      }
    }
  }
  else {
    std::cerr<<"Wrong Data Type\n";
  }
  Bytes b;
  encodeStream_[meta_pos] = {b, data};
  return;
}

void Encoder::EncodePrimitiveType(Data* data) {
  if(data->Type().compare("STRING") == 0) {
    StringEncoding(data);
  }
  else {
    //VarientEncoding(data);
  }
  return;
}

void Encoder::encode(Schema& schema) {
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
          /*
          for(const auto& items : popedElement->Get()) {
            TraverseStack.push(items);
          }
          */
          for(int i = popedElement->Get().size() -1; i >= 0; i--) {
            TraverseStack.push(popedElement->Get()[i]);       
          }
        }
        encodingIdx++;
      }
      for(int i = meta_offset.size()-1; i >= 0 ; i--) {
        std::cout<<"--------> "<<MetaStack.top().first->Type()<<"\n";
        EncodingMeta(MetaStack.top().first, MetaStack.top().second);
        MetaStack.pop();
      }
    }
  }
}

