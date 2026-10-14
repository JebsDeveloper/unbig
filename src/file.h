#pragma once

#include <stdint.h>

typedef struct {
    char magic[4];

    uint64_t compressed_data_size;
    uint64_t decompressed_data_size;

    uint8_t compression_algorithm;
} FileHeader;

uint8_t read_raw_file(const char* path, char** out, uint64_t* out_size);
uint8_t write_raw_file(const char* path, char** in, uint64_t size);

uint8_t write_file(const char* path, FileHeader header, char* compressed_data);
uint8_t read_file(const char* path, FileHeader* header, char** out);
