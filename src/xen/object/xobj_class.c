#include "xobj_class.h"
#include "xobj_string.h"
#include "../xmem.h"

xen_obj_class* xen_obj_class_new(xen_obj_str* name) {
    xen_obj_class* class     = ALLOCATE_OBJ(xen_obj_class, OBJ_CLASS);
    class->name              = name;
    class->properties        = NULL;
    class->property_count    = 0;
    class->property_capacity = 0;
    xen_table_init(&class->methods);
    xen_table_init(&class->private_methods);
    class->initializer        = NULL;
    class->native_initializer = NULL;
    return class;
}

void xen_obj_class_add_property(xen_obj_class* class, xen_obj_str* name, xen_value default_val, bool is_private) {
    if (class->property_count >= class->property_capacity) {
        i32 old_cap              = class->property_capacity;
        class->property_capacity = XEN_GROW_CAPACITY(old_cap);
        class->properties = XEN_GROW_ARRAY(xen_property_def, class->properties, old_cap, class->property_capacity);
    }

    xen_property_def* prop = &class->properties[class->property_count];
    prop->name             = name;
    prop->default_value    = default_val;
    prop->is_private       = is_private;
    prop->index            = class->property_count;
    class->property_count++;
}

void xen_obj_class_add_method(xen_obj_class* class, xen_obj_str* name, xen_obj_func* method, bool is_private) {
    xen_table* table = is_private ? &class->private_methods : &class->methods;
    xen_table_set(table, name, OBJ_VAL(method));
}

// Find property index by name
i32 xen_find_property_index(xen_obj_class* class, xen_obj_str* name) {
    for (i32 i = 0; i < class->property_count; i++) {
        if (class->properties[i].name == name ||
            (class->properties[i].name->length == name->length &&
             memcmp(class->properties[i].name->str, name->str, name->length) == 0)) {
            return i;
        }
    }
    return -1;
}

bool xen_obj_class_is_property_private(xen_obj_class* class, xen_obj_str* name) {
    i32 index = xen_find_property_index(class, name);
    if (index < 0)
        return XEN_FALSE;
    return class->properties[index].is_private;
}

void xen_obj_class_set_native_init(xen_obj_class* class, xen_native_fn init_fn) {
    class->native_initializer = init_fn;
}

void xen_obj_class_add_native_method(xen_obj_class* class, const char* name, xen_native_fn method, bool is_private) {
    xen_obj_str* name_str       = xen_obj_str_copy(name, strlen(name));
    xen_obj_native_func* native = xen_obj_native_func_new(method, name);

    xen_table* table = is_private ? &class->private_methods : &class->methods;
    xen_table_set(table, name_str, OBJ_VAL(native));
}