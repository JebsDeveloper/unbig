#include "compress.h"

#include <stdlib.h>
#include <stdio.h>
#include <lz4.h>

uint8_t compress_lz4(char* in, char** out, uint64_t in_size, uint64_t* out_size) {
    // Allocate the out buffer with the worst-case scenario of bytes for LZ4 compression.
    *out_size = (uint64_t) LZ4_compressBound(in_size);
    *out = (char*) malloc((size_t) *out_size);

    if (*out == NULL) {
        printf("error: failed to allocate %llu bytes of memory.\n", *out_size);
        return 1;
    }

    int result = LZ4_compress_default(in, *out, in_size, *out_size);

    if (result <= 0) {
        printf("error: failed to run LZ4_compress_default (return value %i). Failed to compress data.\n", result);
        free(*out);

        return 1;
    }

    *out_size = (uint64_t) result;

    // Now that it has been compressed, the real size of compressed data is known
    // and the out buffer can be reallocated to not use unnecessary space.
    *out = (char*) realloc(*out, (size_t) result);

    if (*out == NULL) {
        printf("error: failed to reallocate %i bytes of memory for out buffer.\n", result);
        return 1;
    }

    return 0;
}

uint8_t decompress_lz4(char* in, char** out, uint64_t compressed_size, uint64_t decompressed_size) {
    *out = (char*) malloc((size_t) decompressed_size);

    if (*out == NULL) {
        printf("error: failed to allocate %llu bytes of memory.\n", decompressed_size);
        return 1;
    }

    int result = LZ4_decompress_safe(in, *out, compressed_size, decompressed_size);

    if (result < 0) {
        printf("error: failed to decompress data.\n");
        free(*out);

        return 1;
    }

    if (result != decompressed_size) {
        printf("error: the size of decompressed data does not match original metadata. Decompressed %i bytes, original %llu bytes.\n", result, decompressed_size);
        return 1;
    }

    return 0;
}
