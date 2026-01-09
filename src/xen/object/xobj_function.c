#include "xobj_function.h"

xen_obj_func* xen_obj_func_new() {
    xen_obj_func* fn = ALLOCATE_OBJ(xen_obj_func, OBJ_FUNCTION);
    fn->arity        = 0;
    fn->name         = NULL;
    xen_chunk_init(&fn->chunk);
    return fn;
}