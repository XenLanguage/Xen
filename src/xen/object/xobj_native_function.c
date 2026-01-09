#include "xobj_native_function.h"

xen_obj_native_func* xen_obj_native_func_new(xen_native_fn function, const char* name) {
    xen_obj_native_func* fn = ALLOCATE_OBJ(xen_obj_native_func, OBJ_NATIVE_FUNC);
    fn->function            = function;
    fn->name                = name;
    return fn;
}