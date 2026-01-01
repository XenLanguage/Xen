#include "xvalue.h"
#include "xmem.h"
#include "xobject.h"

bool xen_value_equal(xen_value a, xen_value b) {
    if (a.type != b.type)
        return XEN_FALSE;

    switch (a.type) {
        case VAL_BOOL:
            return VAL_AS_BOOL(a) == VAL_AS_BOOL(b);
        case VAL_NUMBER:
            return VAL_AS_NUMBER(a) == VAL_AS_NUMBER(b);
        case VAL_OBJECT:
            return VAL_AS_OBJ(a) == VAL_AS_OBJ(b);
        case VAL_NULL:
            return XEN_TRUE;
        default:
            return XEN_FALSE;
    }
}

void xen_value_array_init(xen_value_array* array) {
    array->count  = 0;
    array->cap    = 0;
    array->values = NULL;
}

void xen_value_array_write(xen_value_array* array, xen_value value) {
    if (array->cap < array->count + 1) {
        const u64 old_cap = array->cap;
        array->cap        = XEN_GROW_CAPACITY(old_cap);
        array->values     = XEN_GROW_ARRAY(xen_value, array->values, old_cap, array->cap);
    }

    array->values[array->count] = value;
    array->count++;
}

void xen_value_array_free(xen_value_array* array) {
    XEN_FREE_ARRAY(xen_value, array->values, array->cap);
    xen_value_array_init(array);
}

void xen_value_print(xen_value value) {
    switch (value.type) {
        case VAL_BOOL:
            printf(VAL_AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NULL:
            printf("null");
            break;
        case VAL_NUMBER:
            printf("%g", VAL_AS_NUMBER(value));
            break;
        case VAL_OBJECT: {
            xen_obj_print(value);
        } break;
        default:
            return;  // Unreachable
    }
}
