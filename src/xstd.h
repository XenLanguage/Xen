#ifndef X_STD_H
#define X_STD_H

#include "xcommon.h"
#include "xvalue.h"

xen_value xstd_println(i32 arg_count, xen_value* args);
xen_value xstd_typeof(i32 arg_count, xen_value* args);

#endif
