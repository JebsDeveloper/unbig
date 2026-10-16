#pragma once

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    char* buffer;
    uint64_t size;
} Buffer;
#pragma pack(pop)
