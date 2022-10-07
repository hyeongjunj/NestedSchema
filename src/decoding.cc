#include "encoding.h"
#include <string>
#include <cstdio>
#include <bitset>
#include <cmath>

/*
Decoding will be done in recursive way.
We have the information in metadata of non-primitive type about
field number and length of fields, etc...
By using that, we can call decoding function recursively.
*/
Schema& Decoder::decode(Bytes &bytes) {
  char* charBytes = reinterpret_cast<char*>(bytes.data());
  int i = 0;
  Struct* data = (Struct*)PartialDecode(charBytes, bytes.size() - 1);
  schema.AddElement("schema", data);
  return schema;
}

// first index indicates the starting point
Data* Decoder::PartialDecode(char* charBytes, int endIdx) {
  Data* data;
  // decodingIdx indicates the starting byte.
  // We have to recursively decode 
  // from [decodingIdx] to [decodingIdx + fieldSize - 1]
  int decodingIdx = 0;
  char type = charBytes[decodingIdx];  // 1 byte for TYPE decoding
  decodingIdx += TYPEMETA;
  int variantLength = 1;
  int numofFields;
  if(type == 7) { // struct
    Struct* str = new Struct();
    numofFields = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    std::vector<std::string> fieldName(numofFields);
    for(int i = 0; i < numofFields; i++) {
      int length = VariantDecoder(charBytes + decodingIdx, decodingIdx);
      fieldName[i] = std::string(charBytes + decodingIdx, length);
      decodingIdx += length;
    }
    std::vector<int64_t> fieldSize(numofFields);
    for(int i = 0; i < numofFields; i++) {
      fieldSize[i] = VariantDecoder(charBytes + decodingIdx, decodingIdx);
    }
    for(int i = 0; i < numofFields; i++) {
      str->add(fieldName[i], PartialDecode(charBytes + decodingIdx, 
                               decodingIdx + fieldSize[i] -1));
      decodingIdx += fieldSize[i];
    }
    data = str;
  }
  else if(type == 8) { // map
    int64_t keyLength   = VariantDecoder(charBytes + decodingIdx, 
				                                 decodingIdx);
    int64_t valueLength = VariantDecoder(charBytes + decodingIdx, 
				                                 decodingIdx);
    Data* Key   = PartialDecode(charBytes + decodingIdx, 
				                        decodingIdx + keyLength   -1);
    decodingIdx += keyLength;
    Data* Value = PartialDecode(charBytes + decodingIdx, 
				                        decodingIdx + valueLength -1);
    data = new Map(Key, Value);
  }
  else if(type == 9) { // array
    Array* arr = new Array();
    numofFields = VariantDecoder(charBytes + decodingIdx, 
				                         decodingIdx);
    std::vector<int64_t> fieldSize(numofFields);
    for(int i = 0; i < numofFields; i++) {
      fieldSize[i] = VariantDecoder(charBytes + decodingIdx, 
					                          decodingIdx);
    }
    for(int i = 0; i < numofFields; i++) {
      arr->Add2Array(PartialDecode(charBytes + decodingIdx, 
						                       decodingIdx + fieldSize[i] -1));
      decodingIdx += fieldSize[i];
    }
    data = arr;
  }
  else {
    data = PrimitiveTypeDecode(charBytes);
  }
  return data;
}

int64_t Decoder::VariantDecoder(char* start_point, int& offset) {
  offset++;
  int idx = 0;
  std::vector<char> charArray;
  while(true) {
    if(!!((start_point[idx] << 0) & 0x80)) {
      charArray.push_back(start_point[idx]);
      idx++; 
      offset++;
    } else {
      charArray.push_back(start_point[idx]);
      break;
    }
  }
  if(idx == 0) return (int64_t)start_point[0];
  for(int i = 0; i < charArray.size() -1; i++) {
    charArray[i] <<= 1;
    charArray[i] >>= 1;
  }
  int bitsNum = charArray.size() * 7;
  int64_t ret = 0;
  for(int i = 0; i < charArray.size(); i++) {
    ret += (int64_t)pow(128, (charArray.size() -1 - i)) 
			   * ((int)charArray[i]);
  }
  return ret;
}

Data* Decoder::PrimitiveTypeDecode(char* charBytes) {
  Data* data;
  int decodingIdx = 0;
  char type = (char)charBytes[decodingIdx];
  decodingIdx++;
  int reserve = decodingIdx; 
  int64_t length = VariantDecoder(charBytes + decodingIdx, 
			                           decodingIdx);
  /*
  VariantDecoder changes decodingIdx but we do not need that 
  when decoding number. So, if this is not type string, 
  going back to previous position.
  */
  if(type == 0) 
    data = new Primitive("I8",     std::to_string(length));
  else if(type == 1)
    data = new Primitive("I16",    std::to_string(length));
  else if(type == 2) 
    data = new Primitive("I32",    std::to_string(length));
  else if(type == 3)  
    data = new Primitive("I64",    std::to_string(length));
  else if(type == 4) 
    data = new Primitive("F32",    std::to_string(length));
  else if(type == 5) 
    data = new Primitive("F64",    std::to_string(length));
  else if(type == 6) 
    data = new Primitive("STRING", 
				     std::string(charBytes + decodingIdx, length));
  else {
    std::cerr<<"ERROR : Failed to Decode\n";
  }
  return data;
}
