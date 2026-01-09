#ifndef XENB_H
#define XENB_H

#include "../xen/xcommon.h"

void xenb_compile(const char* filename, u8* bytecode_out, size_t* size_out);

#endif