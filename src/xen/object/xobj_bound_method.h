#ifndef X_OBJ_BOUND_METHOD_H
#define X_OBJ_BOUND_METHOD_H

#include "xobj.h"
#include "xobj_native_function.h"

struct xen_obj_bound_method {
    xen_obj obj;
    xen_value receiver;    // The object the method is bound to
    xen_native_fn method;  // The native method function
    xen_obj_func* function;
    const char* name;  // Method name for debugging
};

#define OBJ_IS_BOUND_METHOD(v) xen_obj_is_type(v, OBJ_BOUND_METHOD)
#define OBJ_AS_BOUND_METHOD(v) ((xen_obj_bound_method*)VAL_AS_OBJ(v))

xen_obj_bound_method* xen_obj_bound_method_new(xen_value receiver, xen_native_fn method, const char* name);
xen_obj_bound_method* xen_obj_bound_method_new_func(xen_value receiver, xen_obj_func* func);

#endif