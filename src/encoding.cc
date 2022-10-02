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
}

Bytes Encoder::StringEncoding(Data* data) {
}

Bytes Encoder::EncodingMeta(Data* data) {
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
      std::queue<Data*> Queue;
      Queue.push(fields.second);
      while(!Queue.empty()) {
        if(Queue.front()->isPrimitive()) {
          //Encoding(Queue.front());
          std::cout<<Queue.front()->Value()<<"\n";
          Queue.pop();
        } else {
          //EncodingMeta(Queue.front()); // this is nested type,
                               // so we need additional metadata
          Data* popedElement = Queue.front();
          Queue.pop();
          for(const auto& items : popedElement->Get()) {
            //Queue.pop();
            Queue.push(items);
          }
        }
      }
    }
  }
}

