#ifndef X_OBJ_FUNCTION_H
#define X_OBJ_FUNCTION_H

#include "xobj.h"

struct xen_obj_func {
    xen_obj obj;
    i32 arity;  // number of parameters
    xen_chunk chunk;
    xen_obj_str* name;
};

#define OBJ_IS_FUNCTION(v) xen_obj_is_type(v, OBJ_FUNCTION)
#define OBJ_AS_FUNCTION(v) ((xen_obj_func*)VAL_AS_OBJ(v))

xen_obj_func* xen_obj_func_new();

#endif