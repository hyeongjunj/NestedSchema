#include "encoding.h"
#include <string>

// for primitive types
void Encoder::VarientEncoding(Data* data) {
	Bytes encodedBytes;
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
  encodeStream_.push_back({encodedBytes, data});
	return;
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

void Encoder::EncodingMeta(Data* data, std::pair<int,int> offset) {
  std::string data_type = data->Type();
  if(data_type.compare("ARRAY") == 0) {
    // Metadata format : [TYPE][NUMBER OF ELEMENTS][LAST LOCATION]
    std::cout<<"ARRAY META     "<<offset.first<<"\n";
    std::string ElementType = data->Get()[0]->Type();

  } 
  else if(data_type.compare("MAP") == 0) {
  // Metadata format : [TYPE][KEY LOCATION][VALUE LOCATION]
    std::cout<<"MAP META       "<<offset.first<<"\n";
    Data* KeyType   = data->Get()[0]; // Key
    Data* ValueType = data->Get()[1]; // Value
  }
  else if(data_type.compare("STRUCT") == 0) {
  // Metadata foramt : [TYPE][NUMBER OF FILEDS][LOCATION(BYTES)][LOCTION]...
    std::cout<<"STRUCT META    "<<offset.first<<"\n";
  }
  else {
    std::cerr<<"Wrong Data Type\n";
  }
  return;
}

void Encoder::EncodePrimitiveType(Data* data) {
  if(data->Type().compare("STRING") == 0) {
    StringEncoding(data);
  }
  else {
    VarientEncoding(data);
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
      std::stack<Data*> MetaStack;
      //std::stack<std::pair<Data*, int>> EncodingMetaStack;
      TraverseStack.push(fields.second);
      int encodingIdx = 0;
/*
      while(!TraverseStack.empty()) {
        if(TraverseStack.top()->isPrimitive()) {
          std::cout<<TraverseStack.top()->Value()<<" "<<encodingIdx<<"\n";

          EncodePrimitiveType(TraverseStack.top()); 

          TraverseStack.pop();
        } else {
          //EncodingMetaStack.push({TraverseStack.top(),encodingIdx});
          point_.push_back(NestedTypePoint(encodingIdx, TraverseStack.top()));
          std::cout<<"Type : "<<TraverseStack.top()->Type()<<" "<<encodingIdx<<"\n";
          PrepareEncodingMeta(); 
          Data* popedElement = TraverseStack.top();
          TraverseStack.pop();
          for(const auto& items : popedElement->Get()) {
            TraverseStack.push(items);
          }
        }
        encodingIdx++;
      }
*/
      while(!TraverseStack.empty()) {
        if(TraverseStack.top()->isPrimitive()) {
          std::cout<<TraverseStack.top()->Value()<<" "<<encodingIdx<<"\n";
          TraverseStack.pop();
        } else {
          meta_offset_.push_back({encodingIdx, -1}); 
          std::cout<<"Type : "<<TraverseStack.top()->Type()<<" "<<encodingIdx<<"\n";
          PrepareEncodingMeta();
          MetaStack.push(TraverseStack.top()); 
          Data* popedElement = TraverseStack.top();
          TraverseStack.pop();
          for(const auto& items : popedElement->Get()) {
            TraverseStack.push(items);
          }
        }
        encodingIdx++;
      }
      std::cout<<"-----\n";
      /*
      while(!EncodingMetaStack.empty()) {
        EncodingMeta(EncodingMetaStack.top().first, EncodingMetaStack.top().second);
        EncodingMetaStack.pop();
      }
      */
      for(int i = meta_offset_.size()-1; i >= 0 ; i--) {
        EncodingMeta(MetaStack.top(), meta_offset_[i]);
        MetaStack.pop();
      }
      
    }
  }
}

