#include "object.h"

#include "environment.h"
#include "stmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    if (object == NULL) {
        fprintf(stderr, "Aiyo die already lah: Memory allocation fail for the object sia...\n");
        exit(1);
    }
    object->type = type;

    return object;
}

ObjString* copyString(const char* chars, int length) {
    // Allocate memory for the ObjString struct itself PLUS the character array
    // (+1 for null terminator)
    char* heapChars = ALLOCATE(char, length + 1);
    if (heapChars == NULL) return NULL;
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    ObjString* string = (ObjString*)allocateObject(sizeof(ObjString), OBJ_STRING);

    if (string == NULL) {
        // failed, just free
        FREE(char, heapChars);
        return NULL;
    }

    string->length = length;
    string->chars = heapChars;

    return string;
}

ObjArray* newArray(int length) {
    ObjArray* array = (ObjArray*)allocateObject(sizeof(ObjArray), OBJ_ARRAY);
    if (array == NULL) return NULL;

    array->elements = NULL;
    array->length = 0;

    if (length > 0) {
        array->elements = ALLOCATE(Value, length);
        if (array->elements == NULL) {
            FREE(ObjArray, array);
            return NULL;
        }

        for (int i = 0; i < length; i++) {
            array->elements[i] = NIL_VAL;
        }

        array->length = length;
    }

    return array;
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case OBJ_FUNCTION: {
            ObjFunction* function = AS_FUNCTION(value);
            printf("<fn %.*s>",
                   function->declaration->as.function.name.length,
                   function->declaration->as.function.name.start);
            break;
        }
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
        case OBJ_ARRAY: {
            // simple array print
            ObjArray* array = AS_ARRAY(value);
            printf("[");
            for (int i = 0; i < array->length; i++) {
                printValue(array->elements[i]);
                if (i < array->length - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        }
    }
}

void printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "correct" : "wrong");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_OBJ:
            printObject(value);
            break;
    }
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:
            return true;
        case VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ: {
            if (OBJ_TYPE(a) != OBJ_TYPE(b)) return false;

            switch (OBJ_TYPE(a)) {
                case OBJ_STRING: {
                    ObjString* aString = AS_STRING(a);
                    ObjString* bString = AS_STRING(b);
                    // Compare length first for quick check, then content
                    return aString->length == bString->length && memcmp(aString->chars, bString->chars, aString->length) == 0;
                }
                case OBJ_ARRAY: {
                    ObjArray* aArray = AS_ARRAY(a);
                    ObjArray* bArray = AS_ARRAY(b);

                    // quick length sanity-check
                    if (aArray->length != bArray->length) return false;

                    // if same length then we just compare each element
                    for (int i = 0; i < aArray->length; i++) {
                        if (!valuesEqual(aArray->elements[i], bArray->elements[i])) {
                            return false;
                        }
                    }
                    return true;
                }
                default: {
                    printf("Unknown object type: %d\n", OBJ_TYPE(a));
                    return AS_OBJ(a) == AS_OBJ(b);
                }
            }
        }
        default:
            return false;
    }
}

ObjFunction* newFunction(Stmt* declaration, Environment* closure) {
    ObjFunction* function = (ObjFunction*)allocateObject(sizeof(ObjFunction), OBJ_FUNCTION);
    if (function == NULL) return NULL;
    function->declaration = declaration;
    function->closure = closure;
    function->arity = declaration->as.function.param_count;
    return function;
}

ObjNative* newNative(int arity, Value (*function)(struct Interpreter*, int, Value*)) {
    ObjNative* native = (ObjNative*)allocateObject(sizeof(ObjNative), OBJ_NATIVE);
    native->arity = arity;
    native->function = function;
    return native;
}