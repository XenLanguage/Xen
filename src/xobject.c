#include "xobject.h"
#include "xalloc.h"
#include "xtable.h"
#include "xutils.h"
#include "xvalue.h"
#include "xvm.h"

#define ALLOCATE_OBJ(type, obj_type) (type*)allocate_obj(sizeof(type), obj_type);

bool xen_obj_is_type(xen_value value, xen_obj_type type) {
    return VAL_IS_OBJ(value) && VAL_AS_OBJ(value)->type == type;
}

static void print_function(xen_obj_func* fn) {
    if (fn->name == NULL) {
        printf("<script>");
        return;
    }
    printf("<Function %s>", fn->name->str);
}

void xen_obj_print(xen_value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING: {
            const char* val = OBJ_AS_CSTRING(value);
            printf("%s", val);
        } break;
        case OBJ_FUNCTION: {
            print_function(OBJ_AS_FUNCTION(value));
        } break;
        case OBJ_NATIVE_FUNC: {
            printf("<Function xstd::%s>", OBJ_AS_NATIVE_FUNC(value)->name);
        } break;
    }
}

static xen_obj* allocate_obj(size_t size, xen_obj_type type) {
    xen_obj* obj = (xen_obj*)xen_mem_realloc(NULL, 0, size);
    obj->type    = type;
    obj->next    = g_vm.objects;
    g_vm.objects = obj;
    return obj;
}

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

xen_obj_func* xen_obj_func_new() {
    xen_obj_func* fn = ALLOCATE_OBJ(xen_obj_func, OBJ_FUNCTION);
    fn->arity        = 0;
    fn->name         = NULL;
    xen_chunk_init(&fn->chunk);
    return fn;
}

xen_obj_native_func* xen_obj_native_func_new(xen_native_fn function, const char* name) {
    xen_obj_native_func* fn = ALLOCATE_OBJ(xen_obj_native_func, OBJ_NATIVE_FUNC);
    fn->function            = function;
    fn->name                = name;
    return fn;
}
