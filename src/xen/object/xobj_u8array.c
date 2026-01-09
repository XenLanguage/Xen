#include "xobj_u8array.h"
#include "../xmem.h"

xen_obj_u8array* xen_obj_u8array_new() {
    xen_obj_u8array* obj = ALLOCATE_OBJ(xen_obj_u8array, OBJ_U8ARRAY);
    obj->values          = NULL;
    obj->capacity        = 0;
    obj->count           = 0;
    return obj;
}

xen_obj_u8array* xen_obj_u8array_new_with_capacity(i32 capacity) {
    xen_obj_u8array* obj = ALLOCATE_OBJ(xen_obj_u8array, OBJ_U8ARRAY);
    obj->values          = (u8*)malloc(capacity);
    obj->capacity        = capacity;
    obj->count           = 0;
    return obj;
}

void xen_obj_u8array_push(xen_obj_u8array* arr, u8 value) {
    if (arr->capacity < arr->count + 1) {
        const i32 old_cap = arr->capacity;
        arr->capacity     = XEN_GROW_CAPACITY(old_cap);
        arr->values       = XEN_GROW_ARRAY(u8, arr->values, old_cap, arr->capacity);
    }
    arr->values[arr->count] = value;
    arr->count++;
}

u8 xen_obj_u8array_get(xen_obj_u8array* arr, i32 index) {
    if (index > arr->count - 1) {
        xen_runtime_error("index out of bounds (size: %d, index: %d)", arr->count - 1, index);
        return 0;
    }
    u8 value = arr->values[index];
    return value;
}

void xen_obj_u8array_set(xen_obj_u8array* arr, i32 index, u8 value) {
    if (index > arr->count - 1) {
        xen_runtime_error("index out of bounds (size: %d, index: %d)", arr->count - 1, index);
        return;
    }
    arr->values[index] = value;
}

u8 xen_u8obj_array_pop(xen_obj_u8array* arr) {
    if (arr->count == 0)
        return 0;
    arr->count--;
    return arr->values[arr->count];
}

i32 xen_obj_u8array_length(xen_obj_u8array* arr) {
    return arr->count;
}
