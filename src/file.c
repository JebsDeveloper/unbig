#include "file.h"

#include <stdlib.h>
#include <stdio.h>

uint8_t read_raw_file(const char* path, Buffer* buffer) {
    // Open the file in binary read mode.
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        printf("error: failed to open file at '%s' for binary read.\n", path);
        return 1;
    }

    // Seek to the end of the file to determine its length.
    if (fseek(file, 0L, SEEK_END) != 0) {
        printf("error: failed to seek end of file '%s'.\n", path);
        fclose(file);
        
        return 1;
    }

    int64_t size = ftell(file);

    if (size < 0) {
        printf("error: failed to get size (return value %lld) of file '%s'.\n", (long long int) size, path);
        fclose(file);

        return 1;
    }

    buffer->size = size;

    if (fseek(file, 0L, SEEK_SET) != 0) {
        printf("error: failed to seek beginning of file '%s'.\n", path);
        fclose(file);

        return 1;
    }

    buffer->buffer = (char*) malloc((size_t) size);

    if (buffer->buffer == NULL) {
        printf("error: failed to allocate %llu bytes of memory for file '%s'.\n", (long long unsigned int) size, path);
        fclose(file);

        return 1;
    }

    if (fread(buffer->buffer, size, 1, file) != 1) {
        printf("error: failed to read %llu bytes of data from file '%s'.\n", (long long unsigned int) size, path);

        free(buffer->buffer);
        fclose(file);
            
        return 1;
    }

    fclose(file);
    return 0;
}

uint8_t write_raw_file(const char* path, Buffer* buffer) {
    // Open the file in binary write mode.
    FILE* file = fopen(path, "wb");

    if (file == NULL) {
        printf("error: failed to open file at '%s' for binary write.\n", path);
        return 1;
    }

    // fwrite should return exactly what was passed in its 3rd parameter because it
    // is how many times that data has been written to the file.
    if (fwrite(buffer->buffer, buffer->size, 1, file) != 1) {
        printf("error: failed to write compressed data to file '%s'.\n", path);
        fclose(file);

        return 1;
    }

    fclose(file);
    return 0;
}

uint8_t write_file(const char* path, FileHeader header, char* compressed_data) {
    // Open the file in binary write mode.
    FILE* file = fopen(path, "wb");

    if (file == NULL) {
        printf("error: failed to open file at '%s' for binary write.\n", path);
        return 1;
    }

    // fwrite should return exactly what was passed in its 3rd parameter because it
    // is how many times that data has been written to the file.
    if (fwrite(&header, sizeof(FileHeader), 1, file) != 1) {
        printf("error: failed to write header to file '%s'.\n", path);
        fclose(file);

        return 1;
    }

    if (fwrite(compressed_data, header.compressed_data_size, 1, file) != 1) {
        printf("error: failed to write compressed data to file '%s'.\n", path);
        fclose(file);

        return 1;
    }

    fclose(file);
    return 0;
}

uint8_t read_file(const char* path, FileHeader* header, char** out) {
    // Open the file in read binary mode.
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        printf("error: failed to open file at '%s' for binary read.\n", path);
        return 1;
    }

    // Same as with fwrite, fread returns the amount of times it read that data which
    // should be one since the 3rd parameter is set to 1.
    if (fread(header, sizeof(FileHeader), 1, file) != 1) {
        printf("error: failed to read header from file '%s'.\n", path);
        fclose(file);
        
        return 1;
    }

    *out = (char*) malloc((size_t) header->compressed_data_size);

    if (*out == NULL) {
        printf("error: failed to allocate %llu bytes of memory.\n", (long long unsigned int) header->compressed_data_size);
        fclose(file);

        return 1;
    }

    if (fread(*out, header->compressed_data_size, 1, file) != 1) {
        printf("error: failed to read compressed data from file '%s'.\n", path);

        free(*out);
        fclose(file);

        return 1;
    }

    fclose(file);
    return 0;
}
