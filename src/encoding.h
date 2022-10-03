#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <bitset>
#include <queue>
#include <stack>
#include <iostream>
#include "schema.h"
/*
typedef struct NestedTypePoint_ {
  int64_t start_;
  int64_t end_;
  int64_t encodingIdx_;
  Data* data_;
  NestedTypePoint_(int encodingIdx, Data* data) :
                  encodingIdx_(encodingIdx),
                  data_(data) {}
} NestedTypePoint;
*/

class Encoder {
private:
  std::vector<std::pair<Bytes, Data*>> encodeStream_;
  std::vector<std::pair<int, int>> meta_offset_;
  // <encodingIdx, endpoint> 
public:
  void encode(Schema& schema);
  //Bytes Encoding(Data* data);
  void VarientEncoding(Data* data);
  void StringEncoding(Data* data);
  void PrepareEncodingMeta();
  void EncodingMeta(Data* data, std::pair<int,int> offset);
  void EncodePrimitiveType(Data* data);
};