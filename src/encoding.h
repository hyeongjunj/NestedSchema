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
public:
  Data* PartialDecode(int s, int e, char* charBytes);
  Data* PrimitiveTypeDecode(int s, int e, char* charBytes);
  Data* StructTypeDecode(int s, int e, char* charBytes);
  Data* MapTypeDecode(int s, int e, char* charBytes);
  Data* ArrayTypeDecode(int s, int e, char* charBytes);
  int VariantDecoder(char* start_point, int offset); 
  Schema& decode(Bytes &bytes);
};