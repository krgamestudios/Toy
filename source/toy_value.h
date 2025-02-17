#pragma once

#include "toy_common.h"
#include "toy_print.h"

//forward declarations
struct Toy_Bucket;
union Toy_String_t;
struct Toy_Array;
struct Toy_Table;
union Toy_Function_t;

typedef enum Toy_ValueType {
	TOY_VALUE_NULL,
	TOY_VALUE_BOOLEAN,
	TOY_VALUE_INTEGER,
	TOY_VALUE_FLOAT,
	TOY_VALUE_STRING,
	TOY_VALUE_ARRAY,
	TOY_VALUE_TABLE,
	TOY_VALUE_FUNCTION,
	TOY_VALUE_OPAQUE,
	TOY_VALUE_ANY,

	TOY_VALUE_REFERENCE, //not a value itself, but pointing to one
	TOY_VALUE_UNKNOWN, //The correct type is unknown, but will be determined later
} Toy_ValueType;

//8 bytes in size
typedef struct Toy_Value {             //32 | 64 BITNESS
	union {
		struct Toy_Value* reference;   //4  | 8
		bool boolean;                  //1  | 1
		int integer;                   //4  | 4
		float number;                  //4  | 4
		union Toy_String_t* string;    //4  | 8
		struct Toy_Array* array;       //4  | 8
		struct Toy_Table* table;       //4  | 8
		union Toy_Function_t* function;//4  | 8
		//TODO: more types go here

	} as;                              //4  | 8

	Toy_ValueType type;                //4  | 4
} Toy_Value;                           //8  | 16

#define TOY_VALUE_IS_NULL(value)				((value).type == TOY_VALUE_NULL)
#define TOY_VALUE_IS_BOOLEAN(value)				((value).type == TOY_VALUE_BOOLEAN)
#define TOY_VALUE_IS_INTEGER(value)				((value).type == TOY_VALUE_INTEGER)
#define TOY_VALUE_IS_FLOAT(value)				((value).type == TOY_VALUE_FLOAT)
#define TOY_VALUE_IS_STRING(value)				((value).type == TOY_VALUE_STRING)
#define TOY_VALUE_IS_ARRAY(value)				((value).type == TOY_VALUE_ARRAY || (TOY_VALUE_IS_REFERENCE(value) && Toy_unwrapValue(value).type == TOY_VALUE_ARRAY))
#define TOY_VALUE_IS_TABLE(value)				((value).type == TOY_VALUE_TABLE || (TOY_VALUE_IS_REFERENCE(value) && Toy_unwrapValue(value).type == TOY_VALUE_TABLE))
#define TOY_VALUE_IS_FUNCTION(value)			((value).type == TOY_VALUE_FUNCTION)
#define TOY_VALUE_IS_OPAQUE(value)				((value).type == TOY_VALUE_OPAQUE)
#define TOY_VALUE_IS_TYPE(value)				((value).type == TOY_VALUE_TYPE)
#define TOY_VALUE_IS_REFERENCE(value)			((value).type == TOY_VALUE_REFERENCE)

#define TOY_VALUE_AS_BOOLEAN(value)				((value).as.boolean)
#define TOY_VALUE_AS_INTEGER(value)				((value).as.integer)
#define TOY_VALUE_AS_FLOAT(value)				((value).as.number)
#define TOY_VALUE_AS_STRING(value)				((value).as.string)
#define TOY_VALUE_AS_ARRAY(value)				((TOY_VALUE_IS_REFERENCE(value) ? Toy_unwrapValue(value) : value).as.array)
#define TOY_VALUE_AS_TABLE(value)				((TOY_VALUE_IS_REFERENCE(value) ? Toy_unwrapValue(value) : value).as.table)
#define TOY_VALUE_AS_FUNCTION(value)			((value).as.function)
//TODO: more

#define TOY_VALUE_FROM_NULL()					((Toy_Value){{ .integer = 0 }, TOY_VALUE_NULL})
#define TOY_VALUE_FROM_BOOLEAN(value)			((Toy_Value){{ .boolean = value }, TOY_VALUE_BOOLEAN})
#define TOY_VALUE_FROM_INTEGER(value)			((Toy_Value){{ .integer = value }, TOY_VALUE_INTEGER})
#define TOY_VALUE_FROM_FLOAT(value)				((Toy_Value){{ .number = value }, TOY_VALUE_FLOAT})
#define TOY_VALUE_FROM_STRING(value)			((Toy_Value){{ .string = value }, TOY_VALUE_STRING})
#define TOY_VALUE_FROM_ARRAY(value)				((Toy_Value){{ .array = value }, TOY_VALUE_ARRAY})
#define TOY_VALUE_FROM_TABLE(value)				((Toy_Value){{ .table = value }, TOY_VALUE_TABLE})
#define TOY_VALUE_FROM_FUNCTION(value)			((Toy_Value){{ .function = value }, TOY_VALUE_FUNCTION})
//TODO: more

#define TOY_REFERENCE_FROM_POINTER(ptr)			((Toy_Value){{ .reference = ptr }, TOY_VALUE_REFERENCE})

//utilities
TOY_API Toy_Value Toy_unwrapValue(Toy_Value value);
TOY_API unsigned int Toy_hashValue(Toy_Value value);

TOY_API Toy_Value Toy_copyValue(Toy_Value value);
TOY_API Toy_Value Toy_deepCopyValue(struct Toy_Bucket** bucketHandle, Toy_Value value); //don't use refcounting
TOY_API void Toy_freeValue(Toy_Value value);

TOY_API bool Toy_checkValueIsTruthy(Toy_Value value);
TOY_API bool Toy_checkValuesAreEqual(Toy_Value left, Toy_Value right);
TOY_API bool Toy_checkValuesAreComparable(Toy_Value left, Toy_Value right);
TOY_API int Toy_compareValues(Toy_Value left, Toy_Value right);

//convert the value to a string - values that *are* strings are simply copied
TOY_API union Toy_String_t* Toy_stringifyValue(struct Toy_Bucket** bucketHandle, Toy_Value value);

//for debugging
TOY_API const char* Toy_private_getValueTypeAsCString(Toy_ValueType type);
