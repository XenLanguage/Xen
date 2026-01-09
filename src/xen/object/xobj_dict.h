#ifndef X_OBJ_DICT_H
#define X_OBJ_DICT_H

#include "xobj.h"
#include "../builtin/xbuiltin_dict.h"

struct xen_obj_dict {
    xen_obj obj;
    xen_table table;
};

#define OBJ_IS_DICT(value) xen_obj_is_type(value, OBJ_DICT)
#define OBJ_AS_DICT(value) ((xen_obj_dict*)VAL_AS_OBJ(value))

xen_obj_dict* xen_obj_dict_new();
void xen_obj_dict_set(xen_obj_dict* dict, xen_value key, xen_value value);
bool xen_obj_dict_get(xen_obj_dict* dict, xen_value key, xen_value* out);
bool xen_obj_dict_delete(xen_obj_dict* dict, xen_value key);

static xen_method_entry k_dict_methods[] = {{"len", xen_dict_len, XEN_TRUE},  // property
                                            {"keys", xen_dict_keys, XEN_FALSE},
                                            {"values", xen_dict_values, XEN_FALSE},
                                            {"has", xen_dict_has, XEN_FALSE},
                                            {"remove", xen_dict_remove, XEN_FALSE},
                                            {"clear", xen_dict_clear, XEN_FALSE},
                                            {NULL, NULL, XEN_FALSE}};

#endif