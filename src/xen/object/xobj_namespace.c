#include "xobj_namespace.h"
#include "../xmem.h"

xen_obj_namespace* xen_obj_namespace_new(const char* name) {
    xen_obj_namespace* ns = ALLOCATE_OBJ(xen_obj_namespace, OBJ_NAMESPACE);
    ns->name              = name;
    ns->entries           = NULL;
    ns->count             = 0;
    ns->capacity          = 0;
    return ns;
}

void xen_obj_namespace_set(xen_obj_namespace* ns, const char* name, xen_value value) {
    for (i32 i = 0; i < ns->count; i++) {
        if (strcmp(ns->entries[i].name, name) == 0) {
            ns->entries[i].value = value;
            return;
        }
    }

    if (ns->count >= ns->capacity) {
        i32 old_cap  = ns->capacity;
        ns->capacity = XEN_GROW_CAPACITY(old_cap);
        ns->entries  = XEN_GROW_ARRAY(xen_ns_entry, ns->entries, old_cap, ns->capacity);
    }

    ns->entries[ns->count].name  = name;
    ns->entries[ns->count].value = value;
    ns->count++;
}

bool xen_obj_namespace_get(xen_obj_namespace* ns, const char* name, xen_value* out) {
    for (i32 i = 0; i < ns->count; i++) {
        if (strcmp(ns->entries[i].name, name) == 0) {
            *out = ns->entries[i].value;
            return XEN_TRUE;
        }
    }
    return XEN_FALSE;
}