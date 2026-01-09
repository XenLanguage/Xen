#include "xobj_string.h"
#include "../xmem.h"
#include "../xutils.h"
#include "../xvm.h"

static xen_obj_str* allocate_str(char* chars, i32 length, u32 hash) {
    xen_obj_str* str = ALLOCATE_OBJ(xen_obj_str, OBJ_STRING);
    str->length      = length;
    str->str         = chars;
    str->hash        = hash;
    xen_table_set(&g_vm.strings, str, NULL_VAL);
    return str;
}

xen_obj_str* xen_obj_str_take(char* chars, i32 length) {
    u32 hash              = xen_hash_string(chars, length);
    xen_obj_str* interned = xen_table_find_str(&g_vm.strings, chars, length, hash);
    if (interned != NULL) {
        XEN_FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocate_str(chars, length, hash);
}

xen_obj_str* xen_obj_str_copy(const char* chars, i32 length) {
    u32 hash              = xen_hash_string(chars, length);
    xen_obj_str* interned = xen_table_find_str(&g_vm.strings, chars, length, hash);
    if (interned != NULL)
        return interned;

    char* heap_chars = XEN_ALLOCATE(char, length + 1);
    if (!heap_chars) {
        xen_panic(XEN_ERR_ALLOCATION_FAILED, "failed to allocate heap memory for string");
    }

    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';

    return allocate_str(heap_chars, length, hash);
}