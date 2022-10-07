# Binary Format for encoding

In this schema, all elements can be categorized as 3 types : 
1. Struct
2. Map
3. Array
4. String
5. Variant Number

Each elements needs its own metadata, and I decided to put metadata first, and element data second.

E.g., [METADATA] [ELEMENT DATA] [METADATA] [ELEMENT DATA] [METADATA] [ELEMENT DATA] [METADATA] [ELEMENT DATA] ...

## Struct
For struct, we need following metadata
1. Type(Let decoder knows the type) for 1 byte
2. Number of fields (Variant)
3. Field Names (Varaint type of length integer + String)
4. Sizes of each fields (Variant)
For example,

Struct {

  field_1 : data
  
  field_2 : data
  
  field_3 : data
  
} 

For this struct, we need this kind of metadata.

[ Type ][ How many fields? ][  field_1  ][  field_2  ][  field_3  ][ field_1 size ][ field_2 size ][ field_3 size ]

Note that except "Type", size can vary.

## Map
For map, we need following metadata
1. Type (1 byte)
2. Size of each fields (Variant)

[Type][ key size ][ value size ]


## Array
For array, we need following metadata
1. Type(Let decoder knows the type) for 1 byte
2. Number of fields (Variant)
4. Sizes of each fields (Variant)

[ Type ][ How many fields? ][ field_1 size ][ field_2 size ][ field_3 size ][ field_3 size ][ field_3 size ] ...

## String
For string, we need type, length and string data.

[ Type ][ Length ][        String        ]

## Others
Variant integer and other numbers will be stored without any metadata.
