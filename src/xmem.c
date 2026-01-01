#include "xmem.h"
#include "xerr.h"
#include "xvalue.h"
#include "xobject.h"
#include "xvm.h"

void xen_vm_mem_init(xen_vm_mem* mem, size_t size_perm, size_t size_gen, size_t size_temp) {
    mem->permanent  = xen_alloc_create(size_perm);
    mem->generation = xen_alloc_create(size_gen);
    mem->temporary  = xen_alloc_create(size_temp);
}

void xen_vm_mem_destroy(xen_vm_mem* mem) {
    xen_alloc_destroy(mem->permanent);
    xen_alloc_destroy(mem->generation);
    xen_alloc_destroy(mem->temporary);
}

void* xen_mem_realloc(void* ptr, size_t old_size, size_t new_size) {
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }

    void* result = realloc(ptr, new_size);
    if (result == NULL)
        xen_panic(XEN_ERR_ALLOCATION_FAILED, "failed to reallocate memory");

    return result;
}

void xen_mem_free_objects() {
    xen_obj* obj = g_vm.objects;
    while (obj != NULL) {
        xen_obj* next = obj->next;
        xen_mem_free_object(obj);
        obj = next;
    }
}

static void xen_mem_free_object(xen_obj* obj) {
    switch (obj->type) {
        case OBJ_STRING: {
            const xen_obj_str* str = (xen_obj_str*)obj;
            XEN_FREE_ARRAY(char, str->str, str->length + 1);
            XEN_FREE(xen_obj_str, obj);
            break;
        }
    }
}
