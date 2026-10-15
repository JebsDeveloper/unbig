#include "compress.h"

#include <stdlib.h>
#include <stdio.h>
#include <lz4.h>

uint8_t compress_lz4(Buffer* in, Buffer* out) {
    // Allocate the out buffer with the worst-case scenario of bytes for LZ4 compression.
    out->size = (uint64_t) LZ4_compressBound(in->size);
    out->buffer = (char*) malloc((size_t) out->size);

    if (out->buffer == NULL) {
        printf("error: failed to allocate %llu bytes of memory.\n", out->size);
        return 1;
    }

    int result = LZ4_compress_default(in->buffer, out->buffer, in->size, out->size);

    if (result <= 0) {
        printf("error: failed to run LZ4_compress_default (return value %i). Failed to compress data.\n", result);
        free(out->buffer);

        return 1;
    }

    out->size = (uint64_t) result;

    // Now that it has been compressed, the real size of compressed data is known
    // and the out buffer can be reallocated to not use unnecessary space.
    out->buffer = (char*) realloc(out->buffer, (size_t) result);

    if (out->buffer == NULL) {
        printf("error: failed to reallocate %i bytes of memory for out buffer.\n", result);
        free(out->buffer);
        
        return 1;
    }

    return 0;
}

uint8_t decompress_lz4(Buffer* in, Buffer* out) {
    out->buffer = (char*) malloc((size_t) out->size);

    if (out->buffer == NULL) {
        printf("error: failed to allocate %llu bytes of memory.\n", out->size);
        return 1;
    }

    int result = LZ4_decompress_safe(in->buffer, out->buffer, in->size, out->size);

    if (result < 0) {
        printf("error: failed to decompress data.\n");
        free(out->buffer);

        return 1;
    }

    if (result != out->size) {
        printf("error: the size of decompressed data does not match original metadata. Decompressed %i bytes, original %llu bytes.\n", result, out->size);
        free(out->buffer);
        
        return 1;
    }

    return 0;
}
