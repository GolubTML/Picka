#include <android/log.h>
#include <dlfcn.h>

#define TAG "PureInjector"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

__attribute__((constructor))
void init()
{
    LOGI("We are in! Loading payload..");

    void* handle = dlopen("/data/data/com.and.games505.TerrariaPaid/files/payload.so", RTLD_NOW); // для теста пусть будет в корневой папке
    if (!handle)
    {
        LOGI("Failed to load payload.so! %s", dlerror());
        return;
    }

    void (*payload_init)() = dlsym(handle, "payload_init");
    if (payload_init)
    {
        payload_init();
    }

    LOGI("Payload loaded!");
}