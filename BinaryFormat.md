# Binary Format for encoding
In this schema, all elements can be categorized as 5 types : 
* Struct
* Map
* Array
* String
* Variant Number

Each elements needs its own metadata, and I decided to put metadata first, and element data second.
```
[METADATA] [ELEMENT DATA] [METADATA] [ELEMENT DATA] [METADATA] [ELEMENT DATA] [METADATA] [ELEMENT DATA] ...
```
## Struct
For struct, we need following metadata
* Type(Let decoder knows the type) for 1 byte
* Number of fields (Variant)
* Field Names (Varaint type of length integer + String)
* Sizes of each fields (Variant)
For example,
```
Struct {

  field_1 : data
  
  field_2 : data
  
  field_3 : data
  
} 
```
For this struct, we need this kind of metadata.

```
[ Type ][ How many fields? ][  field_1  ][  field_2  ][  field_3  ][ field_1 size ][ field_2 size ][ field_3 size ]
```

Note that except "Type", size can vary.

If it has nested type, format will be like this :
E.g.,
```
Struct {

  field_1 : Array : String
                    String
  field_2 : String
  
} 
```
```
[Type(Struct)][ 2 ][ field_1 ][ field_2 ][ field_1 size ][ field_2 size ][Type(Array)][ 2 ][ string size ][ string size ][ length ][ string ]
```

## Map

For map, we need following metadata
* Type (1 byte)
* Size of each fields (Variant)
```
[Type][ key size ][ value size ]
```

## Array

For array, we need following metadata
* Type(Let decoder knows the type) for 1 byte
* Number of fields (Variant)
* Sizes of each fields (Variant)
```
[ Type ][ How many fields? ][ field_1 size ][ field_2 size ][ field_3 size ][ field_3 size ][ field_3 size ] ...
```
## String

For string, we need type, length and string data.
```
[ Type ][ Length ][        String        ]
```
## Others

Variant integer and other numbers will be stored without any metadata.

## Use of Variant

We can easily learn that 
```
0 0 0 0 1 1 0
```
indicates 6.

But the number can go beyond 1 bytes.

So, by using MSB(Most Significant Bit), we can store variant size of numbers without additional metadata(E.g., length of numbers).

### Encode Example
```
1 1 1 1 0 0 1 1 (243)
```
#### Step 1
```
0 0 0 0 0 0 1 / 1 1 1 0 0 1 1
```

Devide it with 7 bits.

#### Step 2
Put MSB except for the rightmost.

Insert 0 to the rightmost.
```
1 0 0 0 0 0 0 1 / 0 1 1 1 0 0 1 1
```
### Decode Example

#### Step 1

Take out MSB bits.
```
1 0 0 0 0 0 0 1 / 0 1 1 1 0 0 1 1
```
```
_ 0 0 0 0 0 0 1 / _ 1 1 1 0 0 1 1
```
#### Step 2

Concatenate it
```
_ 0 0 0 0 0 0 1 + _ 1 1 1 0 0 1 1
```
```
  0 0 0 0 0 0 1 1 1 1 0 0 1 1
```  
Now we can discover this was 243.

