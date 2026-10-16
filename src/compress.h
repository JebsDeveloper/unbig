#pragma once

#include <stdint.h>
#include "utils.h"

// Compression algorithms.
enum {
    COMPRESS_NONE = 0,
    COMPRESS_LZ4 = 1
};

// Thank you to the creators of this amazing LZ4 compression library!
// The LZ4 folder in include is a clone of the GitHub repository:
// https://github.com/lz4/lz4

uint8_t compress_lz4(Buffer* in, Buffer* out);
uint8_t decompress_lz4(Buffer* in, Buffer* out);

uint8_t stream_compress_lz4(const char* input_file, const char* output_file);
