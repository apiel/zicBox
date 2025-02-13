// apt-get install libduktape207 duktape-dev

#include <string>
#include <vector>

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

To set config from a js script, you need to call `zic(key, value);` in the js script:

```js
zic("print", "Call ZicBox print from Lua.")
zic("PLUGIN_COMPONENT", "Encoder2 ./plugins/components/build/libzic_Encoder2Component.so")
zic("LOAD_HOST", "config.cfg")
zic("VIEW", "Main")

// Let's create an encoder function
function setEncoder(x, y, width, height, encoder_id, value) {
  zic("COMPONENT", "Encoder2 " + x + " " + y + " " + width + " " + height);
  zic("ENCODER_ID", encoder_id);
  zic("VALUE", value);
}

setEncoder(10, 10, 100, 100, 1, "FM DECAY_1");

// ...
```

Finally run zicBox like this:
```sh
./zicBox config.js plugins/config/build/libzic_JsConfig.so
```

> [!NOTE]
> Duktape is a subset of JavaScript and might not support all features.

*/

struct Var {
    std::string key;
    std::string value;
};

struct UserData {
    void (*callback)(char* command, char* params, const char* filename, std::vector<Var> variables);
    const char* filename;
    std::vector<Var> variables;
};

duk_ret_t zicSetConfigFn(duk_context* ctx)
{
    std::string key = duk_to_string(ctx, 0);
    std::string value = duk_to_string(ctx, 1);

    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("userDataPtr"));
    UserData* userData = (UserData*)(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    // printf("-------> %s: %s (%s)\n", key, value, userData->filename);

    userData->callback((char*)key.c_str(), (char*)value.c_str(), userData->filename, userData->variables);
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

void config(std::string filename, void (*callback)(char* command, char* params, const char* filename, std::vector<Var> variables), std::vector<Var> variables)
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

    UserData* userData = new UserData();
    userData->callback = callback;
    userData->filename = filename.c_str();
    userData->variables = variables;
    duk_push_global_stash(ctx);
    duk_push_pointer(ctx, (void*)userData);
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("userDataPtr"));

    duk_push_c_function(ctx, zicSetConfigFn, 2);
    duk_put_global_string(ctx, "zic");

#ifdef IS_RPI
    duk_push_boolean(ctx, true);
#else
    duk_push_boolean(ctx, false);
#endif
    duk_put_global_string(ctx, "IS_RPI");

    for (int i = 0; i < variables.size(); i++) {
        duk_push_string(ctx, variables[i].key.c_str());
        duk_put_global_string(ctx, variables[i].value.c_str());
    }

    push_file_as_string(ctx, filename.c_str());
    if (duk_peval(ctx) != 0) {
        printf("Error: %s\n", duk_safe_to_string(ctx, -1));
    }
    duk_pop(ctx);
    duk_destroy_heap(ctx);

    delete userData;
}
}
