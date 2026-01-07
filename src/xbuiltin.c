#include "xbuiltin.h"
#include "xcommon.h"
#include "xerr.h"
#include "xobject.h"
#include "xvalue.h"
#include "xutils.h"
#include "xtypeid.h"
#include <ctype.h>
#include <time.h>

#define REQUIRE_ARG(name, slot, typeid)                                                                                \
    do {                                                                                                               \
        if (argc < slot + 1) {                                                                                         \
            xen_runtime_error("argument '%s' (position %d) required for %s", name, slot, __func__);                    \
            return NULL_VAL;                                                                                           \
        }                                                                                                              \
        if (typeid != xen_typeid_get(argv[slot]) && typeid != TYPEID_UNDEFINED) {                                      \
            xen_runtime_error("expected typeid %d for argment '%s' (got %d"), typeid, name,                            \
              xen_typeid_get(argv[slot]);                                                                              \
            return NULL_VAL;                                                                                           \
        }                                                                                                              \
    } while (XEN_FALSE)

static void define_native_fn(const char* name, xen_native_fn fn) {
    xen_obj_str* key = xen_obj_str_copy(name, strlen(name));
    xen_table_set(&g_vm.globals, key, OBJ_VAL(xen_obj_native_func_new(fn, name)));
}

static xen_value xen_builtin_typeof(i32 argc, xen_value* argv) {
    REQUIRE_ARG("type", 0, TYPEID_UNDEFINED);
    xen_value val = argv[0];
    const char* type_str;

    if (VAL_IS_BOOL(val)) {
        type_str = "bool";
    } else if (VAL_IS_NULL(val)) {
        type_str = "null";
    } else if (VAL_IS_NUMBER(val)) {
        type_str = "number";
    } else if (VAL_IS_OBJ(val)) {
        switch (OBJ_TYPE(val)) {
            case OBJ_STRING:
                type_str = "string";
                break;
            case OBJ_FUNCTION:
                type_str = "function";
                break;
            case OBJ_NATIVE_FUNC:
                type_str = "native_function";
                break;
            case OBJ_NAMESPACE:
                type_str = "namespace";
                break;
            case OBJ_ARRAY:
                type_str = "array";
                break;
            case OBJ_DICT:
                type_str = "dictionary";
                break;
            case OBJ_CLASS:
                type_str = "class";
                break;
            case OBJ_INSTANCE:
                type_str = "instance";
                break;
            case OBJ_BOUND_METHOD:
                type_str = "bound_method";
                break;
            default:
                type_str = "undefined";
                break;
        }
    } else {
        type_str = "undefined";
    }

    return OBJ_VAL(xen_obj_str_copy(type_str, strlen(type_str)));
}

// type constructors
static xen_value xen_builtin_number_ctor(i32 argc, xen_value* argv) {
    REQUIRE_ARG("construct_from", 0, TYPEID_UNDEFINED);
    xen_value val = argv[0];
    switch (val.type) {
        case VAL_BOOL:
            return NUMBER_VAL(VAL_AS_BOOL(val));
        case VAL_NULL:
            return NUMBER_VAL(0);
        case VAL_NUMBER:
            return val;
        case VAL_OBJECT: {
            if (OBJ_IS_STRING(val)) {
                const char* str = OBJ_AS_CSTRING(val);
                f64 num         = strtod(str, NULL);
                return NUMBER_VAL(num);
            } else {
                const char* type_str = xen_value_type_to_str(val);
                xen_runtime_error("cannot construct number from %s", type_str);
                return NULL_VAL;
            }
        }
    }

    return NULL_VAL;
}

static xen_value xen_builtin_string_ctor(i32 argc, xen_value* argv) {
    REQUIRE_ARG("construct_from", 0, TYPEID_UNDEFINED);
    xen_value val = argv[0];
    switch (val.type) {
        case VAL_BOOL: {
            const char* bool_str = VAL_AS_BOOL(val) == XEN_TRUE ? "true" : "false";
            return OBJ_VAL(xen_obj_str_copy(bool_str, strlen(bool_str)));
        }
        case VAL_NULL:
            return OBJ_VAL(xen_obj_str_copy("null", 4));
        case VAL_NUMBER: {
            char buffer[128] = {'\0'};
            snprintf(buffer, sizeof(buffer), "%f", VAL_AS_NUMBER(val));
            return OBJ_VAL(xen_obj_str_copy(buffer, strlen(buffer)));
        }
        case VAL_OBJECT: {
            if (OBJ_IS_STRING(val)) {
                return val;
            }
            break;
        }
    }

    const char* obj_str = xen_value_type_to_str(val);
    return OBJ_VAL(xen_obj_str_copy(obj_str, strlen(obj_str)));
}

static xen_value xen_builtin_bool_ctor(i32 argc, xen_value* argv) {
    REQUIRE_ARG("construct_from", 0, TYPEID_UNDEFINED);
    xen_value val = argv[0];
    switch (val.type) {
        case VAL_BOOL: {
            return val;
        }
        case VAL_NULL:
            return BOOL_VAL(XEN_FALSE);
        case VAL_NUMBER: {
            if (VAL_AS_NUMBER(val) != 0) {
                return BOOL_VAL(XEN_TRUE);
            } else {
                return BOOL_VAL(XEN_FALSE);
            }
        }
        default:
            break;
    }

    return BOOL_VAL(XEN_TRUE);
}

// signature: (number of elements, default value (or null if missing))
static xen_value xen_builtin_array_ctor(i32 argc, xen_value* argv) {
    if (argc > 2) {
        xen_runtime_error("array constructor has invalid number of arguments");
        return NULL_VAL;
    }

    if (argc > 0 && argv[0].type != VAL_NUMBER) {
        xen_runtime_error("element count must be a number");
        return NULL_VAL;
    }

    i32 element_count       = (argc > 0 && VAL_IS_NUMBER(argv[0])) ? VAL_AS_NUMBER(argv[0]) : 0;
    bool has_default        = (argc == 2) ? XEN_TRUE : XEN_FALSE;
    xen_value default_value = (has_default) ? argv[1] : NULL_VAL;

    // create the array
    xen_obj_array* arr = xen_obj_array_new_with_capacity(element_count);
    for (i32 i = 0; i < element_count; i++) {
        xen_obj_array_push(arr, default_value);
    }

    return OBJ_VAL(arr);
}

void xen_builtins_register() {
    srand((u32)time(NULL));
    xen_vm_register_namespace("math", OBJ_VAL(xen_builtin_math()));
    xen_vm_register_namespace("io", OBJ_VAL(xen_builtin_io()));
    xen_vm_register_namespace("string", OBJ_VAL(xen_builtin_string()));
    xen_vm_register_namespace("datetime", OBJ_VAL(xen_builtin_datetime()));
    xen_vm_register_namespace("array", OBJ_VAL(xen_builtin_array()));
    xen_vm_register_namespace("os", OBJ_VAL(xen_builtin_os()));

    // register globals
    define_native_fn("typeof", xen_builtin_typeof);

    // type constructors
    define_native_fn("number", xen_builtin_number_ctor);
    define_native_fn("string", xen_builtin_string_ctor);
    define_native_fn("bool", xen_builtin_bool_ctor);
    define_native_fn("array", xen_builtin_array_ctor);
}

void xen_vm_register_namespace(const char* name, xen_value ns) {
    xen_obj_str* name_str = xen_obj_str_copy(name, (i32)strlen(name));
    xen_table_set(&g_vm.namespace_registry, name_str, ns);
}

// ============================================================================
// Math namespace
// ============================================================================

#include <math.h>

static xen_value math_sqrt(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    return NUMBER_VAL(sqrt(VAL_AS_NUMBER(argv[0])));
}

static xen_value math_sin(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    return NUMBER_VAL(sin(VAL_AS_NUMBER(argv[0])));
}

static xen_value math_cos(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    return NUMBER_VAL(cos(VAL_AS_NUMBER(argv[0])));
}

static xen_value math_tan(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    return NUMBER_VAL(tan(VAL_AS_NUMBER(argv[0])));
}

static xen_value math_pow(i32 argc, xen_value* argv) {
    REQUIRE_ARG("base", 0, TYPEID_NUMBER);
    REQUIRE_ARG("exponent", 1, TYPEID_NUMBER);
    return NUMBER_VAL(pow(VAL_AS_NUMBER(argv[0]), VAL_AS_NUMBER(argv[1])));
}

static xen_value math_log(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    return NUMBER_VAL(log(VAL_AS_NUMBER(argv[0])));
}

static xen_value math_log10(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    return NUMBER_VAL(log10(VAL_AS_NUMBER(argv[0])));
}

static xen_value math_exp(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    return NUMBER_VAL(exp(VAL_AS_NUMBER(argv[0])));
}

static xen_value math_min(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    f64 min = VAL_AS_NUMBER(argv[0]);
    for (i32 i = 1; i < argc; i++) {
        if (VAL_IS_NUMBER(argv[i])) {
            f64 val = VAL_AS_NUMBER(argv[i]);
            if (val < min)
                min = val;
        }
    }
    return NUMBER_VAL(min);
}

static xen_value math_max(i32 argc, xen_value* argv) {
    REQUIRE_ARG("value", 0, TYPEID_NUMBER);
    f64 max = VAL_AS_NUMBER(argv[0]);
    for (i32 i = 1; i < argc; i++) {
        if (VAL_IS_NUMBER(argv[i])) {
            f64 val = VAL_AS_NUMBER(argv[i]);
            if (val > max)
                max = val;
        }
    }
    return NUMBER_VAL(max);
}

static xen_value math_random(i32 argc, xen_value* argv) {
    XEN_UNUSED(argc);
    XEN_UNUSED(argv);
    return NUMBER_VAL((f64)rand() / (f64)RAND_MAX);
}

xen_obj_namespace* xen_builtin_math() {
    xen_obj_namespace* math = xen_obj_namespace_new("math");
    xen_obj_namespace_set(math, "sqrt", OBJ_VAL(xen_obj_native_func_new(math_sqrt, "sqrt")));
    xen_obj_namespace_set(math, "abs", OBJ_VAL(xen_obj_native_func_new(xen_num_abs, "abs")));
    xen_obj_namespace_set(math, "floor", OBJ_VAL(xen_obj_native_func_new(xen_num_floor, "floor")));
    xen_obj_namespace_set(math, "ceil", OBJ_VAL(xen_obj_native_func_new(xen_num_ceil, "ceil")));
    xen_obj_namespace_set(math, "round", OBJ_VAL(xen_obj_native_func_new(xen_num_round, "round")));
    xen_obj_namespace_set(math, "sin", OBJ_VAL(xen_obj_native_func_new(math_sin, "sin")));
    xen_obj_namespace_set(math, "cos", OBJ_VAL(xen_obj_native_func_new(math_cos, "cos")));
    xen_obj_namespace_set(math, "tan", OBJ_VAL(xen_obj_native_func_new(math_tan, "tan")));
    xen_obj_namespace_set(math, "pow", OBJ_VAL(xen_obj_native_func_new(math_pow, "pow")));
    xen_obj_namespace_set(math, "log", OBJ_VAL(xen_obj_native_func_new(math_log, "log")));
    xen_obj_namespace_set(math, "log10", OBJ_VAL(xen_obj_native_func_new(math_log10, "log10")));
    xen_obj_namespace_set(math, "exp", OBJ_VAL(xen_obj_native_func_new(math_exp, "exp")));
    xen_obj_namespace_set(math, "min", OBJ_VAL(xen_obj_native_func_new(math_min, "min")));
    xen_obj_namespace_set(math, "max", OBJ_VAL(xen_obj_native_func_new(math_max, "max")));
    xen_obj_namespace_set(math, "random", OBJ_VAL(xen_obj_native_func_new(math_random, "random")));
    xen_obj_namespace_set(math, "PI", NUMBER_VAL(3.14159265358979323846));
    xen_obj_namespace_set(math, "E", NUMBER_VAL(2.71828182845904523536));
    xen_obj_namespace_set(math, "TAU", NUMBER_VAL(6.28318530717958647692));
    return math;
}

// ============================================================================
// IO namespace
// ============================================================================

static xen_value io_println(i32 argc, xen_value* argv) {
    REQUIRE_ARG("msg", 0, TYPEID_UNDEFINED);
    for (i32 i = 0; i < argc; i++) {
        xen_value_print(argv[i]);
    }
    printf("\n");
    return NULL_VAL;
}

static xen_value io_print(i32 argc, xen_value* argv) {
    REQUIRE_ARG("msg", 0, TYPEID_UNDEFINED);
    for (i32 i = 0; i < argc; i++) {
        xen_value_print(argv[i]);
    }
    return NULL_VAL;
}

static xen_value io_input(i32 argc, xen_value* argv) {
    bool has_prefix = (argc > 0 && argv[0].type == VAL_OBJECT && OBJ_IS_STRING(argv[0]));
    if (has_prefix)
        printf("%s", OBJ_AS_CSTRING(argv[0]));

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        return OBJ_VAL(xen_obj_str_copy(buffer, (i32)len));
    }
    return NULL_VAL;
}

static xen_value io_clear(i32 argc, xen_value* argv) {
    XEN_UNUSED(argc);
    XEN_UNUSED(argv);
    printf("\033[2J\033[H");  // should be at least somewhat cross-platform
    return BOOL_VAL(XEN_TRUE);
}

static xen_value io_pause(i32 argc, xen_value* argv) {
    XEN_UNUSED(argc);
    XEN_UNUSED(argv);
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
    return NUMBER_VAL(c);
}

xen_obj_namespace* xen_builtin_io() {
    xen_obj_namespace* io = xen_obj_namespace_new("io");
    xen_obj_namespace_set(io, "println", OBJ_VAL(xen_obj_native_func_new(io_println, "println")));
    xen_obj_namespace_set(io, "print", OBJ_VAL(xen_obj_native_func_new(io_print, "print")));
    xen_obj_namespace_set(io, "input", OBJ_VAL(xen_obj_native_func_new(io_input, "input")));
    xen_obj_namespace_set(io, "clear", OBJ_VAL(xen_obj_native_func_new(io_clear, "clear")));
    xen_obj_namespace_set(io, "pause", OBJ_VAL(xen_obj_native_func_new(io_pause, "pause")));
    return io;
}

// ============================================================================
// OS namespace
// ============================================================================

#include <sys/stat.h>
#include <errno.h>
#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #include <windows.h>
    #define mkdir(path, mode) _mkdir(path)
    #define rmdir(path) _rmdir(path)
    #define unlink(path) _unlink(path)
#else
    #include <sys/types.h>
    #include <dirent.h>
    #include <unistd.h>
#endif

static xen_value os_readtxt(i32 argc, xen_value* argv) {
    REQUIRE_ARG("filename", 0, TYPEID_STRING);
    xen_value val = argv[0];

    if (val.type == VAL_OBJECT && OBJ_IS_STRING(val)) {
        FILE* fp = fopen(OBJ_AS_CSTRING(val), "r");
        if (!fp) {
            xen_runtime_error("failed to open file: %s", OBJ_AS_CSTRING(val));
            return NULL_VAL;
        }

        fseek(fp, 0, SEEK_END);
        i32 fp_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        i32 input_size = fp_size + 1;
        char* buffer   = (char*)malloc(input_size);

        size_t read_bytes  = fread(buffer, 1, input_size, fp);
        buffer[read_bytes] = '\0';
        fclose(fp);

        return OBJ_VAL(xen_obj_str_take(buffer, input_size));
    } else {
        xen_runtime_error("filename argument must be of type string (got '%s')", xen_value_type_to_str(val));
        return NULL_VAL;
    }
}

static xen_value os_readlines(i32 argc, xen_value* argv) {
    REQUIRE_ARG("filename", 0, TYPEID_STRING);
    xen_value val = argv[0];

    if (val.type == VAL_OBJECT && OBJ_IS_STRING(val)) {
        FILE* fp = fopen(OBJ_AS_CSTRING(val), "r");
        if (!fp) {
            xen_runtime_error("failed to open file: %s", OBJ_AS_CSTRING(val));
            return NULL_VAL;
        }

        xen_obj_array* arr = xen_obj_array_new();

        char* line;
        while ((line = xen_read_line(fp)) != NULL) {
            xen_obj_array_push(arr, OBJ_VAL(xen_obj_str_copy(line, strlen(line))));
            free(line);
        }

        fclose(fp);

        return OBJ_VAL(arr);
    } else {
        xen_runtime_error("filename argument must be of type string (got '%s')", xen_value_type_to_str(val));
        return NULL_VAL;
    }
}

static xen_value os_exit(i32 argc, xen_value* argv) {
    i32 exit_code = 0;
    if (argc > 0 && VAL_IS_NUMBER(argv[0]))
        exit_code = (i32)VAL_AS_NUMBER(argv[0]);

    printf("Xen was terminated with exit code %d\n", exit_code);
    exit(exit_code);

    return NULL_VAL;
}

// Signature:
// os.exec( cmd, argv[] ) -> exit_code
// TODO: This uses system and is quite unsafe, need to refactor to use fork instead
static xen_value os_exec(i32 argc, xen_value* argv) {
    REQUIRE_ARG("cmd", 0, TYPEID_STRING);
    xen_obj_str* cmd = OBJ_AS_STRING(argv[0]);

    i32 args_count = 0;
    if (argc == 2 && OBJ_IS_ARRAY(argv[1])) {
        args_count = OBJ_AS_ARRAY(argv[1])->array.count;
    }

    char cmd_buffer[XEN_STRBUF_SIZE] = {'\0'};
    size_t offset                    = 0;
    memcpy(cmd_buffer, cmd->str, cmd->length);
    cmd_buffer[cmd->length + 1] = ' ';  // add a space after the command
    offset += cmd->length + 1;

    if (args_count > 0) {
        xen_obj_array* args_arr = OBJ_AS_ARRAY(argv[1]);
        for (i32 i = 0; i < args_count; i++) {
            xen_value arg = args_arr->array.values[i];
            if (OBJ_IS_STRING(arg)) {
                const char* arg_str = OBJ_AS_CSTRING(arg);
                memcpy(cmd_buffer + offset, arg_str, strlen(arg_str));
                cmd_buffer[strlen(arg_str) + 1] = ' ';
                offset += strlen(arg_str) + 1;
            }
        }
    }

    i32 exit_code = system(cmd_buffer);
    return NUMBER_VAL(exit_code);
}

static i32 remove_directory(const char* path, bool recursive) {
    if (!recursive) {
        return rmdir(path);
    }

    // Recursive removal
#ifdef _WIN32
    WIN32_FIND_DATA find_data;
    HANDLE hFind;
    char search_path[MAX_PATH];
    char file_path[MAX_PATH];

    snprintf(search_path, MAX_PATH, "%s\\*", path);
    hFind = FindFirstFile(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        return rmdir(path);
    }

    do {
        if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
            snprintf(file_path, MAX_PATH, "%s\\%s", path, find_data.cFileName);

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                remove_directory(file_path, true);  // Recursively remove subdirectories
            } else {
                unlink(file_path);
            }
        }
    } while (FindNextFile(hFind, &find_data));

    FindClose(hFind);
    return rmdir(path);
#else
    DIR* dir;
    struct dirent* entry;
    char file_path[1024];
    struct stat statbuf;

    dir = opendir(path);
    if (!dir) {
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);

        if (stat(file_path, &statbuf) == -1) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            remove_directory(file_path, XEN_TRUE);  // Recursively remove subdirectories
        } else {
            unlink(file_path);
        }
    }

    closedir(dir);
    return rmdir(path);
#endif
}

static bool path_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static bool is_directory(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return XEN_FALSE;
    }
    return S_ISDIR(st.st_mode);
}

static bool is_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return XEN_FALSE;
    }
    return S_ISREG(st.st_mode);
}

// Signature:
// * dir: string
// * should_overwrite: bool
// fn mkdir(dir, should_overwrite) -> success: true, fail: false
static xen_value os_mkdir(int argc, xen_value* argv) {
    REQUIRE_ARG("dir", 0, TYPEID_STRING);
    const char* dir = OBJ_AS_CSTRING(argv[0]);

    bool should_overwrite = XEN_FALSE;
    if (argc > 1 && xen_typeid_get(argv[1]) == TYPEID_BOOL) {
        should_overwrite = VAL_AS_BOOL(argv[1]);
    }

    struct stat st;
    if (stat(dir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            if (should_overwrite) {
                if (remove_directory(dir, XEN_TRUE) != 0) {
                    xen_runtime_error("failed to remove directory '%s'", dir);
                    return BOOL_VAL(XEN_FALSE);
                }
            } else {
                return BOOL_VAL(XEN_FALSE);  // directory exists but no overwrite
            }
        }
    }

#ifdef _WIN32
    int result = mkdir(path);
#else
    int result = mkdir(dir, 0755);
#endif

    if (result == 0) {
        return BOOL_VAL(XEN_TRUE);
    } else {
        return BOOL_VAL(XEN_FALSE);
    }
}

// Signature:
// * dir: string
// * recursive: bool
// fn rmdir(dir, recursive) -> success: true, fail: false
static xen_value os_rmdir(int argc, xen_value* argv) {
    REQUIRE_ARG("dir", 0, TYPEID_STRING);
    const char* dir = OBJ_AS_CSTRING(argv[0]);

    bool recursive = XEN_FALSE;
    if (argc > 1 && xen_typeid_get(argv[1]) == TYPEID_BOOL) {
        recursive = VAL_AS_BOOL(argv[1]);
    }

    i32 result = remove_directory(dir, recursive);
    if (result != 0) {
        xen_runtime_error("failed to remove directory '%s' (not empty)", dir);
        return BOOL_VAL(XEN_FALSE);
    }

    return BOOL_VAL(XEN_TRUE);
}

// Signature:
// * filename: string
// fn rm(filename) -> success: true, fail: false
static xen_value os_rm(i32 argc, xen_value* argv) {
    REQUIRE_ARG("filename", 0, TYPEID_STRING);
    const char* filename = OBJ_AS_CSTRING(argv[0]);

    if (!path_exists(filename)) {
        xen_runtime_error("file does not exist: %s", filename);
        return BOOL_VAL(XEN_FALSE);
    }

    if (!is_file(filename)) {
        xen_runtime_error("path is not a file: %s", filename);
        return BOOL_VAL(XEN_FALSE);
    }

    if (unlink(filename) == 0) {
        return BOOL_VAL(XEN_TRUE);
    } else {
        xen_runtime_error("failed to delete file: %s", filename);
        return BOOL_VAL(XEN_FALSE);
    }
}

// Signature:
// * path: string (file or directory)
// fn exists(dir, recursive) -> bool
static xen_value os_exists(i32 argc, xen_value* argv) {
    REQUIRE_ARG("path", 0, TYPEID_STRING);
    return BOOL_VAL(path_exists(OBJ_AS_CSTRING(argv[0])));
}

xen_obj_namespace* xen_builtin_os() {
    xen_obj_namespace* os = xen_obj_namespace_new("os");
    xen_obj_namespace_set(os, "readtxt", OBJ_VAL(xen_obj_native_func_new(os_readtxt, "readtxt")));
    xen_obj_namespace_set(os, "readlines", OBJ_VAL(xen_obj_native_func_new(os_readlines, "readlines")));
    xen_obj_namespace_set(os, "exit", OBJ_VAL(xen_obj_native_func_new(os_exit, "exit")));
    xen_obj_namespace_set(os, "exec", OBJ_VAL(xen_obj_native_func_new(os_exec, "exec")));
    xen_obj_namespace_set(os, "mkdir", OBJ_VAL(xen_obj_native_func_new(os_mkdir, "mkdir")));
    xen_obj_namespace_set(os, "rmdir", OBJ_VAL(xen_obj_native_func_new(os_rmdir, "rmdir")));
    xen_obj_namespace_set(os, "rm", OBJ_VAL(xen_obj_native_func_new(os_rm, "rm")));
    xen_obj_namespace_set(os, "exists", OBJ_VAL(xen_obj_native_func_new(os_exists, "exists")));
    return os;
}

// ============================================================================
// String methods (exposed for type methods)
// ============================================================================

xen_value xen_str_len(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_STRING(argv[0]))
        return NULL_VAL;
    return NUMBER_VAL(OBJ_AS_STRING(argv[0])->length);
}

xen_value xen_str_upper(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_STRING(argv[0]))
        return NULL_VAL;
    xen_obj_str* str = OBJ_AS_STRING(argv[0]);
    char* buffer     = malloc(str->length + 1);
    for (i32 i = 0; i < str->length; i++) {
        buffer[i] = toupper(str->str[i]);
    }
    buffer[str->length] = '\0';
    return OBJ_VAL(xen_obj_str_take(buffer, str->length));
}

xen_value xen_str_lower(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_STRING(argv[0]))
        return NULL_VAL;
    xen_obj_str* str = OBJ_AS_STRING(argv[0]);
    char* buffer     = malloc(str->length + 1);
    for (i32 i = 0; i < str->length; i++) {
        buffer[i] = tolower(str->str[i]);
    }
    buffer[str->length] = '\0';
    return OBJ_VAL(xen_obj_str_take(buffer, str->length));
}

xen_value xen_str_trim(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_STRING(argv[0]))
        return NULL_VAL;
    xen_obj_str* str = OBJ_AS_STRING(argv[0]);

    const char* start = str->str;
    const char* end   = str->str + str->length - 1;

    while (start <= end && isspace(*start))
        start++;
    while (end > start && isspace(*end))
        end--;

    i32 new_len = (i32)(end - start + 1);
    return OBJ_VAL(xen_obj_str_copy(start, new_len));
}

xen_value xen_str_contains(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_STRING(argv[0]) || !OBJ_IS_STRING(argv[1]))
        return BOOL_VAL(XEN_FALSE);
    xen_obj_str* haystack = OBJ_AS_STRING(argv[0]);
    xen_obj_str* needle   = OBJ_AS_STRING(argv[1]);
    return BOOL_VAL(strstr(haystack->str, needle->str) != NULL);
}

xen_value xen_str_starts_with(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_STRING(argv[0]) || !OBJ_IS_STRING(argv[1]))
        return BOOL_VAL(XEN_FALSE);
    xen_obj_str* str    = OBJ_AS_STRING(argv[0]);
    xen_obj_str* prefix = OBJ_AS_STRING(argv[1]);
    if (prefix->length > str->length)
        return BOOL_VAL(XEN_FALSE);
    return BOOL_VAL(memcmp(str->str, prefix->str, prefix->length) == 0);
}

xen_value xen_str_ends_with(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_STRING(argv[0]) || !OBJ_IS_STRING(argv[1]))
        return BOOL_VAL(XEN_FALSE);
    xen_obj_str* str    = OBJ_AS_STRING(argv[0]);
    xen_obj_str* suffix = OBJ_AS_STRING(argv[1]);
    if (suffix->length > str->length)
        return BOOL_VAL(XEN_FALSE);
    const char* start = str->str + str->length - suffix->length;
    return BOOL_VAL(memcmp(start, suffix->str, suffix->length) == 0);
}

xen_value xen_str_substr(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_STRING(argv[0]) || !VAL_IS_NUMBER(argv[1]))
        return NULL_VAL;

    xen_obj_str* str = OBJ_AS_STRING(argv[0]);
    i32 start        = (i32)VAL_AS_NUMBER(argv[1]);
    i32 len          = (argc >= 3 && VAL_IS_NUMBER(argv[2])) ? (i32)VAL_AS_NUMBER(argv[2]) : str->length - start;

    if (start < 0)
        start = 0;
    if (start >= str->length)
        return OBJ_VAL(xen_obj_str_copy("", 0));
    if (start + len > str->length)
        len = str->length - start;

    return OBJ_VAL(xen_obj_str_copy(str->str + start, len));
}

xen_value xen_str_find(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_STRING(argv[0]) || !OBJ_IS_STRING(argv[1]))
        return NUMBER_VAL(-1);

    xen_obj_str* haystack = OBJ_AS_STRING(argv[0]);
    xen_obj_str* needle   = OBJ_AS_STRING(argv[1]);

    char* found = strstr(haystack->str, needle->str);
    if (found) {
        return NUMBER_VAL(found - haystack->str);
    }
    return NUMBER_VAL(-1);
}

xen_value xen_str_split(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_STRING(argv[0]) || !OBJ_IS_STRING(argv[1]))
        return NULL_VAL;

    xen_obj_str* str   = OBJ_AS_STRING(argv[0]);
    xen_obj_str* delim = OBJ_AS_STRING(argv[1]);

    xen_obj_array* result = xen_obj_array_new();

    if (delim->length == 0) {
        for (i32 i = 0; i < str->length; i++) {
            xen_obj_array_push(result, OBJ_VAL(xen_obj_str_copy(&str->str[i], 1)));
        }
        return OBJ_VAL(result);
    }

    const char* start = str->str;
    const char* end   = str->str + str->length;
    const char* pos;

    while ((pos = strstr(start, delim->str)) != NULL) {
        i32 len = (i32)(pos - start);
        xen_obj_array_push(result, OBJ_VAL(xen_obj_str_copy(start, len)));
        start = pos + delim->length;
    }

    if (start < end) {
        xen_obj_array_push(result, OBJ_VAL(xen_obj_str_copy(start, (i32)(end - start))));
    }

    return OBJ_VAL(result);
}

xen_value xen_str_replace(i32 argc, xen_value* argv) {
    if (argc < 3 || !OBJ_IS_STRING(argv[0]) || !OBJ_IS_STRING(argv[1]) || !OBJ_IS_STRING(argv[2]))
        return argv[0];

    xen_obj_str* str     = OBJ_AS_STRING(argv[0]);
    xen_obj_str* find    = OBJ_AS_STRING(argv[1]);
    xen_obj_str* replace = OBJ_AS_STRING(argv[2]);

    if (find->length == 0)
        return argv[0];

    i32 count       = 0;
    const char* pos = str->str;
    while ((pos = strstr(pos, find->str)) != NULL) {
        count++;
        pos += find->length;
    }

    if (count == 0)
        return argv[0];

    i32 new_len  = str->length + count * (replace->length - find->length);
    char* buffer = malloc(new_len + 1);
    char* dest   = buffer;

    const char* src = str->str;
    while ((pos = strstr(src, find->str)) != NULL) {
        i32 prefix_len = (i32)(pos - src);
        memcpy(dest, src, prefix_len);
        dest += prefix_len;
        memcpy(dest, replace->str, replace->length);
        dest += replace->length;
        src = pos + find->length;
    }

    strcpy(dest, src);

    return OBJ_VAL(xen_obj_str_take(buffer, new_len));
}

xen_obj_namespace* xen_builtin_string() {
    xen_obj_namespace* str = xen_obj_namespace_new("string");
    xen_obj_namespace_set(str, "len", OBJ_VAL(xen_obj_native_func_new(xen_str_len, "len")));
    xen_obj_namespace_set(str, "upper", OBJ_VAL(xen_obj_native_func_new(xen_str_upper, "upper")));
    xen_obj_namespace_set(str, "lower", OBJ_VAL(xen_obj_native_func_new(xen_str_lower, "lower")));
    xen_obj_namespace_set(str, "substr", OBJ_VAL(xen_obj_native_func_new(xen_str_substr, "substr")));
    xen_obj_namespace_set(str, "find", OBJ_VAL(xen_obj_native_func_new(xen_str_find, "find")));
    xen_obj_namespace_set(str, "trim", OBJ_VAL(xen_obj_native_func_new(xen_str_trim, "trim")));
    xen_obj_namespace_set(str, "contains", OBJ_VAL(xen_obj_native_func_new(xen_str_contains, "contains")));
    xen_obj_namespace_set(str, "starts_with", OBJ_VAL(xen_obj_native_func_new(xen_str_starts_with, "starts_with")));
    xen_obj_namespace_set(str, "ends_with", OBJ_VAL(xen_obj_native_func_new(xen_str_ends_with, "ends_with")));
    xen_obj_namespace_set(str, "split", OBJ_VAL(xen_obj_native_func_new(xen_str_split, "split")));
    xen_obj_namespace_set(str, "replace", OBJ_VAL(xen_obj_native_func_new(xen_str_replace, "replace")));
    return str;
}

// ============================================================================
// Time namespace
// ============================================================================

static xen_value time_now(i32 argc, xen_value* argv) {
    XEN_UNUSED(argc);
    XEN_UNUSED(argv);
    return NUMBER_VAL((f64)time(NULL));
}

static xen_value time_clock(i32 argc, xen_value* argv) {
    XEN_UNUSED(argc);
    XEN_UNUSED(argv);
    return NUMBER_VAL((f64)clock() / CLOCKS_PER_SEC);
}

xen_obj_namespace* xen_builtin_datetime() {
    xen_obj_namespace* t = xen_obj_namespace_new("datetime");
    xen_obj_namespace_set(t, "now", OBJ_VAL(xen_obj_native_func_new(time_now, "now")));
    xen_obj_namespace_set(t, "clock", OBJ_VAL(xen_obj_native_func_new(time_clock, "clock")));
    return t;
}

// ============================================================================
// Array methods (exposed for type methods)
// ============================================================================

xen_value xen_arr_len(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_ARRAY(argv[0]))
        return NUMBER_VAL(-1);
    return NUMBER_VAL(OBJ_AS_ARRAY(argv[0])->array.count);
}

xen_value xen_arr_push(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_ARRAY(argv[0]))
        return NULL_VAL;
    xen_obj_array* arr = OBJ_AS_ARRAY(argv[0]);
    for (i32 i = 1; i < argc; i++) {
        xen_obj_array_push(arr, argv[i]);
    }
    return NUMBER_VAL(arr->array.count);
}

xen_value xen_arr_pop(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_ARRAY(argv[0]))
        return NULL_VAL;
    return xen_obj_array_pop(OBJ_AS_ARRAY(argv[0]));
}

xen_value xen_arr_first(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_ARRAY(argv[0]))
        return NULL_VAL;
    xen_obj_array* arr = OBJ_AS_ARRAY(argv[0]);
    if (arr->array.count == 0)
        return NULL_VAL;
    return arr->array.values[0];
}

xen_value xen_arr_last(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_ARRAY(argv[0]))
        return NULL_VAL;
    xen_obj_array* arr = OBJ_AS_ARRAY(argv[0]);
    if (arr->array.count == 0)
        return NULL_VAL;
    return arr->array.values[arr->array.count - 1];
}

xen_value xen_arr_clear(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_ARRAY(argv[0]))
        return NULL_VAL;
    OBJ_AS_ARRAY(argv[0])->array.count = 0;
    return NULL_VAL;
}

xen_value xen_arr_contains(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_ARRAY(argv[0]))
        return BOOL_VAL(XEN_FALSE);
    xen_obj_array* arr = OBJ_AS_ARRAY(argv[0]);
    xen_value needle   = argv[1];
    for (i32 i = 0; i < arr->array.count; i++) {
        if (xen_value_equal(arr->array.values[i], needle)) {
            return BOOL_VAL(XEN_TRUE);
        }
    }
    return BOOL_VAL(XEN_FALSE);
}

xen_value xen_arr_index_of(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_ARRAY(argv[0]))
        return NUMBER_VAL(-1);
    xen_obj_array* arr = OBJ_AS_ARRAY(argv[0]);
    xen_value needle   = argv[1];
    for (i32 i = 0; i < arr->array.count; i++) {
        if (xen_value_equal(arr->array.values[i], needle)) {
            return NUMBER_VAL(i);
        }
    }
    return NUMBER_VAL(-1);
}

xen_value xen_arr_reverse(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_ARRAY(argv[0]))
        return NULL_VAL;
    xen_obj_array* arr = OBJ_AS_ARRAY(argv[0]);
    for (i32 i = 0; i < arr->array.count / 2; i++) {
        i32 j                = arr->array.count - 1 - i;
        xen_value temp       = arr->array.values[i];
        arr->array.values[i] = arr->array.values[j];
        arr->array.values[j] = temp;
    }
    return argv[0];
}

xen_value xen_arr_join(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_ARRAY(argv[0]))
        return NULL_VAL;

    xen_obj_array* arr = OBJ_AS_ARRAY(argv[0]);
    char* delim        = ", ";

    if (argc > 1 && VAL_IS_OBJ(argv[1]) && OBJ_IS_STRING(argv[1])) {
        delim = OBJ_AS_CSTRING(argv[1]);
    }

    const i32 size                     = arr->array.count;
    XEN_CLEANUP_FREE xen_value* values = (xen_value*)malloc(sizeof(xen_value) * size);
    size_t strbuf_size_needed          = 0;

    for (i32 i = 0; i < size; i++) {
        xen_value arg[] = {arr->array.values[i]};
        // use string constructor to handle automatically converting each array value to a string
        xen_value as_str = xen_builtin_string_ctor(1, arg);
        values[i]        = as_str;
        strbuf_size_needed += OBJ_AS_STRING(as_str)->length;
        if (i < size - 1) {
            strbuf_size_needed += strlen(delim);
        }
    }

    XEN_CLEANUP_FREE char* strbuf = (char*)malloc(strbuf_size_needed + 1);
    size_t offset                 = 0;
    for (i32 i = 0; i < size; i++) {
        xen_obj_str* str = OBJ_AS_STRING(values[i]);
        memcpy(strbuf + offset, str->str, str->length);
        offset += str->length;

        if (i < size - 1) {
            memcpy(strbuf + offset, delim, strlen(delim));
            offset += strlen(delim);
        }
    }
    strbuf[strbuf_size_needed] = '\0';

    return OBJ_VAL(xen_obj_str_copy(strbuf, strbuf_size_needed));
}

xen_obj_namespace* xen_builtin_array() {
    xen_obj_namespace* arr = xen_obj_namespace_new("array");
    xen_obj_namespace_set(arr, "len", OBJ_VAL(xen_obj_native_func_new(xen_arr_len, "len")));
    xen_obj_namespace_set(arr, "push", OBJ_VAL(xen_obj_native_func_new(xen_arr_push, "push")));
    xen_obj_namespace_set(arr, "pop", OBJ_VAL(xen_obj_native_func_new(xen_arr_pop, "pop")));
    xen_obj_namespace_set(arr, "first", OBJ_VAL(xen_obj_native_func_new(xen_arr_first, "first")));
    xen_obj_namespace_set(arr, "last", OBJ_VAL(xen_obj_native_func_new(xen_arr_last, "last")));
    xen_obj_namespace_set(arr, "clear", OBJ_VAL(xen_obj_native_func_new(xen_arr_clear, "clear")));
    xen_obj_namespace_set(arr, "contains", OBJ_VAL(xen_obj_native_func_new(xen_arr_contains, "contains")));
    xen_obj_namespace_set(arr, "index_of", OBJ_VAL(xen_obj_native_func_new(xen_arr_index_of, "index_of")));
    xen_obj_namespace_set(arr, "reverse", OBJ_VAL(xen_obj_native_func_new(xen_arr_reverse, "reverse")));
    xen_obj_namespace_set(arr, "join", OBJ_VAL(xen_obj_native_func_new(xen_arr_join, "join")));
    return arr;
}

// ============================================================================
// Number methods (exposed for type methods)
// ============================================================================

xen_value xen_num_abs(i32 argc, xen_value* argv) {
    if (argc < 1 || !VAL_IS_NUMBER(argv[0]))
        return NULL_VAL;
    return NUMBER_VAL(fabs(VAL_AS_NUMBER(argv[0])));
}

xen_value xen_num_floor(i32 argc, xen_value* argv) {
    if (argc < 1 || !VAL_IS_NUMBER(argv[0]))
        return NULL_VAL;
    return NUMBER_VAL(floor(VAL_AS_NUMBER(argv[0])));
}

xen_value xen_num_ceil(i32 argc, xen_value* argv) {
    if (argc < 1 || !VAL_IS_NUMBER(argv[0]))
        return NULL_VAL;
    return NUMBER_VAL(ceil(VAL_AS_NUMBER(argv[0])));
}

xen_value xen_num_round(i32 argc, xen_value* argv) {
    if (argc < 1 || !VAL_IS_NUMBER(argv[0]))
        return NULL_VAL;
    return NUMBER_VAL(round(VAL_AS_NUMBER(argv[0])));
}

xen_value xen_num_to_string(i32 argc, xen_value* argv) {
    if (argc < 1 || !VAL_IS_NUMBER(argv[0]))
        return NULL_VAL;
    char buffer[32];
    i32 len = snprintf(buffer, sizeof(buffer), "%g", VAL_AS_NUMBER(argv[0]));
    return OBJ_VAL(xen_obj_str_copy(buffer, len));
}

// ============================================================================
// Dictionary methods (exposed for type methods)
// ============================================================================

xen_value xen_dict_len(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_DICT(argv[0]))
        return NUMBER_VAL(0);
    xen_obj_dict* dict = OBJ_AS_DICT(argv[0]);
    return NUMBER_VAL(dict->table.count);
}

xen_value xen_dict_keys(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_DICT(argv[0]))
        return NULL_VAL;

    xen_obj_dict* dict  = OBJ_AS_DICT(argv[0]);
    xen_obj_array* keys = xen_obj_array_new();

    for (i32 i = 0; i < dict->table.capacity; i++) {
        xen_table_entry* entry = &dict->table.entries[i];
        if (entry->key != NULL) {
            xen_obj_array_push(keys, OBJ_VAL(entry->key));
        }
    }

    return OBJ_VAL(keys);
}

xen_value xen_dict_values(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_DICT(argv[0]))
        return NULL_VAL;

    xen_obj_dict* dict    = OBJ_AS_DICT(argv[0]);
    xen_obj_array* values = xen_obj_array_new();

    for (i32 i = 0; i < dict->table.capacity; i++) {
        xen_table_entry* entry = &dict->table.entries[i];
        if (entry->key != NULL) {
            xen_obj_array_push(values, entry->value);
        }
    }

    return OBJ_VAL(values);
}

xen_value xen_dict_has(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_DICT(argv[0]))
        return BOOL_VAL(XEN_FALSE);

    xen_obj_dict* dict = OBJ_AS_DICT(argv[0]);
    xen_value dummy;
    return BOOL_VAL(xen_obj_dict_get(dict, argv[1], &dummy));
}

xen_value xen_dict_remove(i32 argc, xen_value* argv) {
    if (argc < 2 || !OBJ_IS_DICT(argv[0]))
        return BOOL_VAL(XEN_FALSE);

    xen_obj_dict* dict = OBJ_AS_DICT(argv[0]);
    return BOOL_VAL(xen_obj_dict_delete(dict, argv[1]));
}

xen_value xen_dict_clear(i32 argc, xen_value* argv) {
    if (argc < 1 || !OBJ_IS_DICT(argv[0]))
        return NULL_VAL;

    xen_obj_dict* dict = OBJ_AS_DICT(argv[0]);
    xen_table_free(&dict->table);
    xen_table_init(&dict->table);
    return NULL_VAL;
}
