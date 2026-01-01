#ifndef X_MEM_H
#define X_MEM_H

#include "xalloc.h"

typedef struct {
    xen_allocator* permanent;   // GC roots, global state
    xen_allocator* generation;  // Current execution generation
    xen_allocator* temporary;   // Expression evaluation, stack frames
} xen_vm_mem;

void xen_vm_mem_init(xen_vm_mem* mem, size_t size_perm, size_t size_gen, size_t size_temp);
void xen_vm_mem_destroy(xen_vm_mem* mem);

typedef struct xen_obj xen_obj;

void* xen_mem_realloc(void* ptr, size_t old_size, size_t new_size);
void xen_mem_free_objects();
static void xen_mem_free_object(xen_obj* obj);

#define XEN_ALLOCATE(type, count) (type*)xen_mem_realloc(NULL, 0, sizeof(type) * (count))
#define XEN_GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)
#define XEN_GROW_ARRAY(type, pointer, old_count, new_count)                                                             \
    (type*)xen_mem_realloc(pointer, sizeof(type) * (old_count), sizeof(type) * (new_count))
#define XEN_FREE_ARRAY(type, pointer, old_count) xen_mem_realloc(pointer, sizeof(type) * (old_count), 0)
#define XEN_FREE(type, pointer) xen_mem_realloc(pointer, sizeof(type), 0)

#endif
