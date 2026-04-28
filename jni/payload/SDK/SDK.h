#pragma once

#include <stdlib.h>

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Vector2 
{
    float x;
    float y;
};

struct Vector3
{
    float x;
    float y;
    float z;
};

namespace SDK
{
    inline void (*NewText)(void* text, uint8_t r, uint8_t g, uint8_t b);

    void Init(uintptr_t base);
    void Chat(const char* message, Color color);
};