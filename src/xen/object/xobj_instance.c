#include "xobj_instance.h"
#include "xobj_class.h"
#include "../xmem.h"

xen_obj_instance* xen_obj_instance_new(xen_obj_class* class) {
    xen_obj_instance* instance = ALLOCATE_OBJ(xen_obj_instance, OBJ_INSTANCE);
    instance->class            = class;

    // Allocate fields and set defaults
    instance->fields = XEN_ALLOCATE(xen_value, class->property_count);
    for (i32 i = 0; i < class->property_count; i++) {
        instance->fields[i] = class->properties[i].default_value;
    }

    return instance;
}

bool xen_obj_instance_get(xen_obj_instance* inst, xen_obj_str* name, xen_value* out) {
    i32 index = xen_find_property_index(inst->class, name);
    if (index >= 0) {
        *out = inst->fields[index];
        return XEN_TRUE;
    }
    return XEN_FALSE;
}

bool xen_obj_instance_set(xen_obj_instance* inst, xen_obj_str* name, xen_value value) {
    i32 index = xen_find_property_index(inst->class, name);
    if (index >= 0) {
        inst->fields[index] = value;
        return XEN_TRUE;
    }
    return XEN_FALSE;
}