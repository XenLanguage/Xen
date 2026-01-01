#include "xstd.h"
#include "xobject.h"

xen_value xstd_println(i32 arg_count, xen_value* args) {
    for (i32 i = 0; i < arg_count; i++) {
        xen_value_print(args[i]);
    }
    printf("\n");
    return NULL_VAL;
}

xen_value xstd_typeof(i32 arg_count, xen_value* args) {
    if (arg_count != 1) {
        return NULL_VAL;
    }

    xen_value val = args[0];
    const char* type_str;

    if (VAL_IS_BOOL(val)) {
        type_str = "bool";
    } else if (VAL_IS_NULL(val)) {
        type_str = "null";
    } else if (VAL_IS_NUMBER(val)) {
        type_str = "number";
    } else if (VAL_IS_OBJ(val)) {
        switch (OBJ_TYPE(val)) {
            case OBJ_STRING:
                type_str = "string";
                break;
            case OBJ_FUNCTION:
                type_str = "function";
                break;
            case OBJ_NATIVE_FUNC:
                type_str = "native_function";
                break;
            default:
                type_str = "undefined";
                break;
        }
    } else {
        type_str = "undefined";
    }

    return OBJ_VAL(xen_obj_str_copy(type_str, strlen(type_str)));
}
