#include <android/log.h>
#include <dlfcn.h>

#define TAG "PureInjector"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

__attribute__((constructor))
void init()
{
    LOGI("We are in! Loading payload..");

    void* handle = dlopen("/data/data/com.and.games505.TerrariaPaid/files/libpayload.so", RTLD_NOW);
    if (!handle)
    {
        const char* error = dlerror();

        LOGI("Failed to load libpayload from files!");
        LOGI("dlopen error: %s", error ? error : "unknown error");

        handle = dlopen("libpayload.so", RTLD_NOW);

        if (!handle)
        {
            error = dlerror();

            LOGI("Failed to load payload.so! %s", dlerror());
            LOGI("dlopen error: %s", error ? error : "unknown error");

            return;
        }   
        else
        {
            LOGI("yep, payload.so in apk");
        }
    }

    void (*payload_init)() = dlsym(handle, "payload_init");
    if (payload_init)
    {
        payload_init();
    }

    LOGI("Payload loaded!");
}