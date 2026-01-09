#ifndef X_OBJ_NATIVE_FUNCTION_H
#define X_OBJ_NATIVE_FUNCTION_H

#include "xobj.h"

struct xen_obj_native_func {
    xen_obj obj;
    xen_native_fn function;
    const char* name;
};

#define OBJ_IS_NATIVE_FUNC(v) xen_obj_is_type(v, OBJ_NATIVE_FUNC)
#define OBJ_AS_NATIVE_FUNC(v) ((xen_obj_native_func*)VAL_AS_OBJ(v))

xen_obj_native_func* xen_obj_native_func_new(xen_native_fn function, const char* name);

#endif