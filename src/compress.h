#pragma once

#include <stdint.h>

// Compression algorithms.
enum {
    COMPRESS_NONE = 0,
    COMPRESS_LZ4 = 1
};

// Thank you to the creators of this amazing LZ4 compression library!
// The LZ4 folder in include is a clone of the GitHub repository:
// https://github.com/lz4/lz4

uint8_t compress_lz4(char* in, char** out, uint64_t in_size, uint64_t* out_size);
uint8_t decompress_lz4(char* in, char** out, uint64_t compressed_size, uint64_t decompressed_size);
