// apt-get install libduktape207 duktape-dev

#include <string>

extern "C" {
#include <duktape.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*md
## JsConfig

JsConfig is a plugin for ZicBox that allows you to set config from a js script. It is based on [Duktape](https://duktape.org/) javascript interpreter.
To use it, you need to install Duktape and Duktape-dev:

```sh
apt-get install libduktape207 duktape-dev
```

To set config from a js script, you need to call `setConfig(key, value);` in the js script:

```js
setConfig("print", "Call ZicBox print from Lua.")
setConfig("PLUGIN_COMPONENT", "Encoder2 ./plugins/components/build/libzic_Encoder2Component.so")
setConfig("LOAD_HOST", "config.cfg")
setConfig("VIEW", "Main")

-- Let's create an encoder function
function setEncoder(x, y, width, height, encoder_id, value) {
  setConfig("COMPONENT", `Encoder2 ${x} ${y} ${width} ${height}`);
  setConfig("ENCODER_ID", encoder_id);
  setConfig("VALUE", value);
}

setEncoder(10, 10, 100, 100, 1, "FM DECAY_1");

// ...
```

Finally run zicBox like this:
```sh
./zicBox config.js plugins/config/build/libzic_JsConfig.so
```
*/

// struct UserData {
//     void (*callback)(char* command, char* params, const char* filename);
//     const char* filename;
// };

duk_ret_t setConfigFn(duk_context* ctx)
{
    const char* key = duk_to_string(ctx, 0);
    const char* value = duk_to_string(ctx, 1);

    // printf("%s: %s\n", key, value);

    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("callbackPtr"));
    void (*callback)(char* command, char* params, const char* filename) = (void (*)(char* command, char* params, const char* filename))(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    // printf("-------> %s: %s (%s)\n", key, value, filename);

    callback((char*)key, (char*)value, "fixme");

    return 0;
}

static void push_file_as_string(duk_context* ctx, const char* filename)
{
    FILE* f;
    size_t len;
    char buf[16384];

    f = fopen(filename, "rb");
    if (f) {
        len = fread((void*)buf, 1, sizeof(buf), f);
        fclose(f);
        duk_push_lstring(ctx, (const char*)buf, (duk_size_t)len);
    } else {
        duk_push_undefined(ctx);
    }
}

void config(std::string filename, void (*callback)(char* command, char* params, const char* filename))
{
    duk_context* ctx = NULL;
    char line[4096];
    size_t idx;
    int ch;

    ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        exit(1);
    }

    duk_push_global_stash(ctx);
    duk_push_pointer(ctx, (void*)callback);
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("callbackPtr"));

    duk_push_c_function(ctx, setConfigFn, 2);
    duk_put_global_string(ctx, "setConfig");

    push_file_as_string(ctx, filename.c_str());
    if (duk_peval(ctx) != 0) {
        printf("Error: %s\n", duk_safe_to_string(ctx, -1));
    }
    duk_pop(ctx);
    duk_destroy_heap(ctx);
}
}