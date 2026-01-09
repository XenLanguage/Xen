#ifndef X_OBJ_ARRAY_H
#define X_OBJ_ARRAY_H

#include "xobj.h"
#include "../builtin/xbuiltin_array.h"

struct xen_obj_array {
    xen_obj obj;
    xen_value_array array;
};

#define OBJ_IS_ARRAY(v) xen_obj_is_type(v, OBJ_ARRAY)
#define OBJ_AS_ARRAY(v) ((xen_obj_array*)VAL_AS_OBJ(v))

xen_obj_array* xen_obj_array_new();
xen_obj_array* xen_obj_array_new_with_capacity(i32 capacity);
void xen_obj_array_push(xen_obj_array* arr, xen_value value);
xen_value xen_obj_array_get(xen_obj_array* arr, i32 index);
void xen_obj_array_set(xen_obj_array* arr, i32 index, xen_value value);
xen_value xen_obj_array_pop(xen_obj_array* arr);
i32 xen_obj_array_length(xen_obj_array* arr);

static xen_method_entry k_array_methods[] = {{"len", xen_arr_len, XEN_TRUE},  // property
                                             {"push", xen_arr_push, XEN_FALSE},
                                             {"pop", xen_arr_pop, XEN_FALSE},
                                             {"first", xen_arr_first, XEN_TRUE},  // property
                                             {"last", xen_arr_last, XEN_TRUE},    // property
                                             {"clear", xen_arr_clear, XEN_FALSE},
                                             {"contains", xen_arr_contains, XEN_FALSE},
                                             {"index_of", xen_arr_index_of, XEN_FALSE},
                                             {"reverse", xen_arr_reverse, XEN_FALSE},
                                             {"join", xen_arr_join, XEN_FALSE},
                                             {NULL, NULL, XEN_FALSE}};

#endif