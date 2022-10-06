#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <bitset>
#include <queue>
#include <stack>
#include <iostream>
#include "schema.h"
#define    TYPEMETA    1
//inline std::unordered_map<int, int> meta_offset;
// <encodingIdx, endpoint>
//inline std::unordered_map<int, int> meta_size_offset;
// <encodingIdx, size>
class Encoder {
private:
  std::vector<std::pair<Bytes, Data*>> encodeStream_;
  std::unordered_map<int, int> meta_offset;
  std::unordered_map<int, int> meta_size_offset;
  
  // <encodingIdx, endpoint> 
public:
  Bytes encode(Schema& schema);
  //Bytes Encoding(Data* data);
  Bytes VarientEncoding(int64_t value);
  Bytes TypeEncoding(Data* data);
  void StringEncoding(Data* data);
  void PrepareEncodingMeta();
  void EncodingMeta(Data* data, int meta_pos);
  void EncodePrimitiveType(Data* data);
};

class Decoder {
private:
  Schema schema;
public:
  Data* PartialDecode(char* charBytes, int endIdx);
  Data* PrimitiveTypeDecode(char* charBytes);
  int VariantDecoder(char* start_point, int& offset); 
  Schema& decode(Bytes &bytes);
};