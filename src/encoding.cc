#include "encoding.h"

Bytes Encoder::Encoding(Data* data) {
  if(data->Type().compare("STRING")) {
    return StringEncoding(data);
  } else {
    return VarientEncoding(data);
  }
}

// for primitive types
Bytes Encoder::VarientEncoding(Data* data) {
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
  encodeStream_.push_back(encodedBytes);
	return encodedBytes;
}

Bytes Encoder::StringEncoding(Data* data) {
  // for string type, length + string bytecode
  // length should be encodeded with VarientEncoding
}

void Encoder::PrepareEncodingMeta() {
  encodeStream_.emplace_back();
  // This will later be filled by EncodingMeta()
}

Bytes Encoder::EncodingMeta(Data* data, int encodingIdx) {

  Bytes encodedBytes;
}

void Encoder::encode(Schema& schema) {
  // we have to traverse the schema for encoding 
  for(const auto& fields : schema.Get()) {
    // fields.first is field name,
    // fields.second is value(Data type)
      
    if(fields.second->isPrimitive()) { // end of the traversal tree
      std::cout<<fields.second->Value()<<"\n";
      //Encoding(fields.second);
    } else { // we have to traverse deep into this data type
             // struct, map, list type
      //std::cout<<"Type : "<<fields.second->Type()<<"\n";
      std::queue<Data*> Queue; 
      std::stack<std::pair<Data*, int>> Stack;
      Queue.push(fields.second);

      int encodingIdx = 0;
      while(!Queue.empty()) {
        if(Queue.front()->isPrimitive()) {
          //Encoding(Queue.front());
          std::cout<<Queue.front()->Value()<<"\n";
          Queue.pop();
        } else {
          Stack.push({Queue.front(),encodingIdx});

          std::cout<<"Type : "<<Queue.front()->Type()<<"\n";

          Data* popedElement = Queue.front();
          Queue.pop();
          for(const auto& items : popedElement->Get()) {
            Queue.push(items);
          }
        }
        encodingIdx++;
      }
      while(!Stack.empty()) {
        EncodingMeta(Stack.top().first, Stack.top().second);
        Stack.pop();
      }
    }
  }
}

