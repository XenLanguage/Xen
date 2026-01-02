#ifndef X_OBJECT_H
#define X_OBJECT_H

#include "xvalue.h"
#include "xchunk.h"

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE_FUNC,
    OBJ_NAMESPACE,
    OBJ_ARRAY,
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

struct xen_ns_entry {
    const char* name;
    xen_value value;
};

struct xen_obj_namespace {
    xen_obj obj;
    const char* name;
    xen_ns_entry* entries;
    i32 count;
    i32 capacity;
};

struct xen_obj_array {
    xen_obj obj;
    xen_value_array array;
};

#define OBJ_TYPE(v) (VAL_AS_OBJ(v)->type)

#define OBJ_IS_STRING(v) xen_obj_is_type(v, OBJ_STRING)
#define OBJ_AS_STRING(v) ((xen_obj_str*)VAL_AS_OBJ(v))
#define OBJ_AS_CSTRING(v) (((xen_obj_str*)VAL_AS_OBJ(v))->str)

#define OBJ_IS_FUNCTION(v) xen_obj_is_type(v, OBJ_FUNCTION)
#define OBJ_AS_FUNCTION(v) ((xen_obj_func*)VAL_AS_OBJ(v))

#define OBJ_IS_NATIVE_FUNC(v) xen_obj_is_type(v, OBJ_NATIVE_FUNC)
#define OBJ_AS_NATIVE_FUNC(v) ((xen_obj_native_func*)VAL_AS_OBJ(v))

#define OBJ_IS_NAMESPACE(v) xen_obj_is_type(v, OBJ_NAMESPACE)
#define OBJ_AS_NAMESPACE(v) ((xen_obj_namespace*)VAL_AS_OBJ(v))

#define OBJ_IS_ARRAY(v) xen_obj_is_type(v, OBJ_ARRAY)
#define OBJ_AS_ARRAY(v) ((xen_obj_array*)VAL_AS_OBJ(v))

bool xen_obj_is_type(xen_value value, xen_obj_type type);
void xen_obj_print(xen_value value);

xen_obj_str* xen_obj_str_take(char* chars, i32 length);
xen_obj_str* xen_obj_str_copy(const char* chars, i32 length);

xen_obj_func* xen_obj_func_new();
xen_obj_native_func* xen_obj_native_func_new(xen_native_fn function, const char* name);

xen_obj_namespace* xen_obj_namespace_new(const char* name);
void xen_obj_namespace_set(xen_obj_namespace* ns, const char* name, xen_value value);
bool xen_obj_namespace_get(xen_obj_namespace* ns, const char* name, xen_value* out);

xen_obj_array* xen_obj_array_new();
xen_obj_array* xen_obj_array_new_with_capacity(i32 capacity);
void xen_obj_array_push(xen_obj_array* arr, xen_value value);
xen_value xen_obj_array_get(xen_obj_array* arr, i32 index);
void xen_obj_array_set(xen_obj_array* arr, i32 index, xen_value value);
xen_value xen_obj_array_pop(xen_obj_array* arr);
i32 xen_obj_array_length(xen_obj_array* arr);

#endif
