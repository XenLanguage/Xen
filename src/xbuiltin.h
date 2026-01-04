#ifndef X_BUILTIN_H
#define X_BUILTIN_H

#include "xcommon.h"
#include "xvm.h"

void xen_builtins_register();
void xen_vm_register_namespace(const char* name, xen_value ns);

xen_obj_namespace* xen_builtin_math();
xen_obj_namespace* xen_builtin_io();
xen_obj_namespace* xen_builtin_string();
xen_obj_namespace* xen_builtin_datetime();
xen_obj_namespace* xen_builtin_array();

/* ============================================================================
 * exposed method functions (shared between namespaces and type methods)
 * ============================================================================ */

// String methods
xen_value xen_str_len(i32 argc, xen_value* args);
xen_value xen_str_upper(i32 argc, xen_value* args);
xen_value xen_str_lower(i32 argc, xen_value* args);
xen_value xen_str_trim(i32 argc, xen_value* args);
xen_value xen_str_contains(i32 argc, xen_value* args);
xen_value xen_str_starts_with(i32 argc, xen_value* args);
xen_value xen_str_ends_with(i32 argc, xen_value* args);
xen_value xen_str_substr(i32 argc, xen_value* args);
xen_value xen_str_find(i32 argc, xen_value* args);
xen_value xen_str_split(i32 argc, xen_value* args);
xen_value xen_str_replace(i32 argc, xen_value* args);

// Array methods
xen_value xen_arr_len(i32 argc, xen_value* args);
xen_value xen_arr_push(i32 argc, xen_value* args);
xen_value xen_arr_pop(i32 argc, xen_value* args);
xen_value xen_arr_first(i32 argc, xen_value* args);
xen_value xen_arr_last(i32 argc, xen_value* args);
xen_value xen_arr_clear(i32 argc, xen_value* args);
xen_value xen_arr_contains(i32 argc, xen_value* args);
xen_value xen_arr_index_of(i32 argc, xen_value* args);
xen_value xen_arr_reverse(i32 argc, xen_value* args);
xen_value xen_arr_join(i32 argc, xen_value* args);

// Number methods
xen_value xen_num_abs(i32 argc, xen_value* args);
xen_value xen_num_floor(i32 argc, xen_value* args);
xen_value xen_num_ceil(i32 argc, xen_value* args);
xen_value xen_num_round(i32 argc, xen_value* args);
xen_value xen_num_to_string(i32 argc, xen_value* args);

#endif
