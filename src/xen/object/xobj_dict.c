#include "xobj_dict.h"
#include "xobj_string.h"

xen_obj_dict* xen_obj_dict_new() {
    xen_obj_dict* dict = ALLOCATE_OBJ(xen_obj_dict, OBJ_DICT);
    xen_table_init(&dict->table);
    return dict;
}

void xen_obj_dict_set(xen_obj_dict* dict, xen_value key, xen_value value) {
    // For now, only allow string keys (simplest approach)
    if (!OBJ_IS_STRING(key)) {
        xen_runtime_error("dictionary keys must be strings");
        return;
    }
    xen_obj_str* key_str = OBJ_AS_STRING(key);
    xen_table_set(&dict->table, key_str, value);
}

bool xen_obj_dict_get(xen_obj_dict* dict, xen_value key, xen_value* out) {
    if (!OBJ_IS_STRING(key)) {
        return XEN_FALSE;
    }
    xen_obj_str* key_str = OBJ_AS_STRING(key);
    return xen_table_get(&dict->table, key_str, out);
}

bool xen_obj_dict_delete(xen_obj_dict* dict, xen_value key) {
    if (!OBJ_IS_STRING(key)) {
        return XEN_FALSE;
    }
    xen_obj_str* key_str = OBJ_AS_STRING(key);
    return xen_table_delete(&dict->table, key_str);
}