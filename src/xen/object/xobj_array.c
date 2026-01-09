#include "xobj_array.h"
#include "../xmem.h"

xen_obj_array* xen_obj_array_new() {
    return xen_obj_array_new_with_capacity(8);
}

xen_obj_array* xen_obj_array_new_with_capacity(i32 capacity) {
    xen_obj_array* arr = ALLOCATE_OBJ(xen_obj_array, OBJ_ARRAY);
    xen_value_array_init(&arr->array);
    if (capacity > 0) {
        arr->array.values = XEN_ALLOCATE(xen_value, capacity);
    }
    return arr;
}

void xen_obj_array_push(xen_obj_array* arr, xen_value value) {
    xen_value_array_write(&arr->array, value);
}

xen_value xen_obj_array_get(xen_obj_array* arr, i32 index) {
    if (index < 0 || index >= arr->array.count) {
        return NULL_VAL;
    }
    return arr->array.values[index];
}

void xen_obj_array_set(xen_obj_array* arr, i32 index, xen_value value) {
    if (index < 0 || index >= arr->array.count) {
        return;  // out of bounds - silently fail for now
    }
    arr->array.values[index] = value;
}

xen_value xen_obj_array_pop(xen_obj_array* arr) {
    if (arr->array.count == 0)
        return NULL_VAL;
    arr->array.count--;
    return arr->array.values[arr->array.count];
}

i32 xen_obj_array_length(xen_obj_array* arr) {
    return arr->array.count;
}