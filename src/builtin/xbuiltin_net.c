#include "xbuiltin_net.h"
#include "xbuiltin_common.h"

xen_obj_namespace* xen_builtin_net() {
    xen_obj_namespace* net = xen_obj_namespace_new("net");
    return net;
}
