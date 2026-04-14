#pragma once

#include <stdlib.h>

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};


namespace SDK
{
    inline void (*NewText)(void* text, uint8_t r, uint8_t g, uint8_t b);

    void Init(uintptr_t base);
    void Chat(const char* message, Color color);
};