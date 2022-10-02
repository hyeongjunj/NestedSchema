#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <bitset>
#include <queue>
#include <stack>
#include <iostream>
#include "schema.h"

class Encoder {
private:
  //Bytes header_;
  //Bytes metadata_;
  //Bytes rawdata_;
  std::vector<Bytes> encodeStream_;
public:
  void encode(Schema& schema);
  Bytes Encoding(Data* data);
  Bytes VarientEncoding(Data* data);
  Bytes StringEncoding(Data* data);
  void PrepareEncodingMeta();
  Bytes EncodingMeta(Data* data, int encodingIdx);
};