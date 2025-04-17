#ifndef sg_object_h
#define sg_object_h

#include <stdbool.h>
#include <stddef.h>

typedef struct Obj Obj;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ  // this is for heap-allocated objects
} ValueType;

// Value structure - represents any runtime value
typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;  // Pointer to heap-allocated object
    } as;
} Value;

typedef enum { OBJ_STRING } ObjType;

struct Obj {
    ObjType type;
};

typedef struct {
    Obj obj;
    int length;
    char* chars;
} ObjString;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) (IS_OBJ(value) && OBJ_TYPE(value) == OBJ_STRING)

// we can cast the obj to an ObjString* and then access the chars
// AS C_STRING returns char* so we can pass it directly to C functions for conv
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

ObjString* copyString(const char* chars, int length);

// Print function for Objects (called by printValue)
void printObject(Value value);

// Function to print a Value (for debugging/printing)
void printValue(Value value);

// Function to check equality (needed later for interpreter)
bool valuesEqual(Value a, Value b);

#endif