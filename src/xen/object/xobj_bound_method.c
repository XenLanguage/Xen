#include "xobj_bound_method.h"
#include "xobj_function.h"
#include "xobj_string.h"

xen_obj_bound_method* xen_obj_bound_method_new(xen_value receiver, xen_native_fn method, const char* name) {
    xen_obj_bound_method* bound = ALLOCATE_OBJ(xen_obj_bound_method, OBJ_BOUND_METHOD);
    bound->receiver             = receiver;
    bound->method               = method;
    bound->name                 = name;
    bound->function             = NULL;
    return bound;
}

xen_obj_bound_method* xen_obj_bound_method_new_func(xen_value receiver, xen_obj_func* func) {
    xen_obj_bound_method* bound = ALLOCATE_OBJ(xen_obj_bound_method, OBJ_BOUND_METHOD);
    bound->receiver             = receiver;
    bound->method               = NULL;
    bound->function             = func;
    bound->name                 = func->name ? func->name->str : "<anonymous>";
    return bound;
}