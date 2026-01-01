#ifndef X_OBJECT_H
#define X_OBJECT_H

#include "xvalue.h"
#include "xchunk.h"

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE_FUNC,
} xen_obj_type;

struct xen_obj {
    xen_obj_type type;
    xen_obj* next;
};

struct xen_obj_str {
    xen_obj obj;
    i32 length;
    char* str;
    u32 hash;
};

struct xen_obj_func {
    xen_obj obj;
    i32 arity;  // number of parameters
    xen_chunk chunk;
    xen_obj_str* name;
};

typedef xen_value (*xen_native_fn)(i32 arg_count, xen_value* args);

struct xen_obj_native_func {
    xen_obj obj;
    xen_native_fn function;
    const char* name;
};

#define OBJ_TYPE(v) (VAL_AS_OBJ(v)->type)

/* String object macros */
#define OBJ_IS_STRING(v) xen_obj_is_type(v, OBJ_STRING)
#define OBJ_AS_STRING(v) ((xen_obj_str*)VAL_AS_OBJ(v))
#define OBJ_AS_CSTRING(v) (((xen_obj_str*)VAL_AS_OBJ(v))->str)

/* Function object macros */
#define OBJ_IS_FUNCTION(v) xen_obj_is_type(v, OBJ_FUNCTION)
#define OBJ_AS_FUNCTION(v) ((xen_obj_func*)VAL_AS_OBJ(v))

#define OBJ_IS_NATIVE_FUNC(v) xen_obj_is_type(v, OBJ_NATIVE_FUNC)
#define OBJ_AS_NATIVE_FUNC(v) ((xen_obj_native_func*)VAL_AS_OBJ(v))

bool xen_obj_is_type(xen_value value, xen_obj_type type);
void xen_obj_print(xen_value value);

xen_obj_str* xen_obj_str_take(char* chars, i32 length);
xen_obj_str* xen_obj_str_copy(const char* chars, i32 length);

xen_obj_func* xen_obj_func_new();
xen_obj_native_func* xen_obj_native_func_new(xen_native_fn function, const char* name);

#endif
