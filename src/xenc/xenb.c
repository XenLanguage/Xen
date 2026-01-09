#include "xenb.h"
#include "xbin_writer.h"

#include "../xen/object/xobj_function.h"
#include "../xen/object/xobj_native_function.h"
#include "../xen/object/xobj_string.h"
#include "../xen/xchunk.h"
#include "../xen/xutils.h"
#include "../xen/xcompiler.h"

/*
 * Bytecode Binary Format (.xenb)
 *
 * MAGIC             : 'XENB'    - 4 bytes
 * Version           : u8        - 1 byte
 * Lines             : u32       - 4 bytes
 * Entrypoint Length : u32       - 4 bytes
 * Entrypoint        : <string>  - n bytes
 * Args Count        : u32       - 4 bytes
 * Constants Size    : u32       - 4 bytes
 * Constants Table   : entry[]   - n bytes
 *   Constant Entry
 *     Type          : u8        - 1 byte
 *     Value Length  : u32       - 4 bytes
 *     Value         : <any>     - n bytes
 * Bytecode Size     : u32       - 4 bytes
 * Bytecode          : u8[]      - n bytes
 */

#define WRITE(v) xen_bin_write(&writer, v)

void xenb_compile(const char* filename, u8* bytecode_out, size_t* size_out) {
    char* source     = xen_read_file(filename);
    xen_obj_func* fn = xen_compile(source);

    if (!fn) {
        xen_panic(XEN_ERR_EXEC_COMPILE, "failed to compile");
    }

    // write function metadata (constants, line count)
    xen_bin_writer writer;
    xen_bin_writer_init(&writer, XEN_MB(4));

    const u8 version            = 1;
    const u32 line_count        = (u32)*fn->chunk.lines;
    const u32 args_count        = (u32)fn->arity;
    const u32 constants_size    = (u32)fn->chunk.constants.count;
    const char* entrypoint_name = fn->name->str;
    const u32 entrypoint_length = (u32)fn->name->length;

    xen_bin_write_u8(&writer, 'X');
    xen_bin_write_u8(&writer, 'E');
    xen_bin_write_u8(&writer, 'N');
    xen_bin_write_u8(&writer, 'B');
    WRITE(version);
    WRITE(line_count);
    xen_bin_write_fixed_str(&writer, entrypoint_name, entrypoint_length + 1);
    WRITE(args_count);
    WRITE(constants_size);

    /*
     * Constant Entry
     *   Type          : u8        - 1 byte
     *   Value Length  : u32       - 4 bytes
     *   Value         : <any>     - n bytes
     */
    for (int i = 0; i < fn->chunk.constants.count; i++) {
        xen_value constant = fn->chunk.constants.values[i];
        WRITE((u8)constant.type);
        switch (constant.type) {
            case VAL_BOOL: {
                WRITE((u32)sizeof(bool));
                WRITE(constant.as.boolean);
            } break;
            case VAL_NUMBER: {
                WRITE((u32)sizeof(f64));
                WRITE(constant.as.number);
            } break;
            case VAL_OBJECT: {
                if (OBJ_IS_STRING(constant)) {
                    xen_obj_str* str = OBJ_AS_STRING(constant);
                    WRITE((u32)str->length);
                    xen_bin_write_fixed_str(&writer, str->str, strlen(str->str) + 1);
                } else if (OBJ_IS_FUNCTION(constant)) {
                    // I don't know if I need to write these or not. I don't think I do.
                } else if (OBJ_IS_NATIVE_FUNC(constant)) {
                }
            } break;
            case VAL_NULL: {
                WRITE((u32)sizeof(bool));
                WRITE(constant.as.boolean);
            } break;
        }
    }

    xen_bin_write_byte_array(&writer, fn->chunk.code, fn->chunk.capacity);

    *size_out = writer.consumed;
    memcpy(bytecode_out, writer.data, writer.consumed);

    free(source);
    xen_bin_writer_free(&writer);
}

#undef WRITE