#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <bitset>
#include <queue>
#include <stack>
#include <iostream>
#include "schema.h"
inline std::unordered_map<int, int> meta_offset;
// <encodingIdx, endpoint>
inline std::unordered_map<int, int> meta_size_offset;
// <encodingIdx, size>
class Encoder {
private:
  std::vector<std::pair<Bytes, Data*>> encodeStream_;
  //std::vector<std::pair<int, int>> meta_offset_;
  //std::unordered_map<int, int> meta_offset_;
  // <encodingIdx, endpoint> 
public:
  void encode(Schema& schema);
  //Bytes Encoding(Data* data);
  void VarientEncoding(Data* data);
  void StringEncoding(Data* data);
  void PrepareEncodingMeta();
  void EncodingMeta(Data* data, int meta_pos);
  void EncodePrimitiveType(Data* data);
};