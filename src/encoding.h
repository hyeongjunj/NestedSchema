#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <bitset>
#include <queue>
#include <iostream>
#include "schema.h"

class Encoder {
private:
  Bytes header_;
  Bytes metadata_;
  Bytes rawdata_;
public:
  void encode(Schema& schema);
  Bytes Encoding(Data* data);
  Bytes VarientEncoding(Data* data);
  Bytes StringEncoding(Data* data);
  Bytes EncodingMeta(Data* data);
};

