#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "compress.h"
#include "file.h"

// Next argument.
enum {
    NEXTARG_NONE = 0,
    NEXTARG_INPUT = 1,
    NEXTARG_OUTPUT = 2,
    NEXTARG_ALGORITHM = 3
};

int main(int argc, char* argv[]) {
    // Argument 1 is the command so if there are no more arguments, none
    // have been passed to the program.
    if (argc < 2) {
        printf("error: no arguments provided (use -h for help)\n");
        return 0;
    }

    if (argc > UINT8_MAX) {
        printf("error: too many arguments (max: 255)\n");
        return 0;
    }

    uint8_t argument_count = (uint8_t) argc;

    // Right now, I have no idea what the conventions are on this, I'm just copying the style
    // of pacman and yay and their help menus and error messages. It's working so far.

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        printf("Usage:\n    unbig <args> <files>\nArgs:\n    unbig {-h, --help} Displays this help menu\n    unbig {-a, --algorithm} <files> Choses which compression algorithm to use.\n");
        return 0;
    }

    // Value is 0 for decompress, 1 for compress, and 2 for unchosen, will be replaced with bitpacking.
    uint8_t compress = 2;
    uint8_t has_input = 0;
    uint8_t has_output = 0;
    uint8_t next_arg = NEXTARG_NONE;

    const char* input_file_path;
    const char* output_file_path;
    uint8_t compression_algorithm = COMPRESS_NONE;

    // Parse the program arguments.
    for (uint8_t i = 1; i < argument_count; ++i) {
        if (next_arg == NEXTARG_NONE) {
            if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--algorithm") == 0) {
                next_arg = NEXTARG_ALGORITHM;
            } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
                next_arg = NEXTARG_INPUT;
            } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
                next_arg = NEXTARG_OUTPUT;
            } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compress") == 0) {
                compress = 1;
            } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decompress") == 0) {
                compress = 0;
            }

            continue;
        }

        switch (next_arg) {
            case NEXTARG_INPUT: {
                input_file_path = argv[i];
                has_input = 1;

                next_arg = NEXTARG_NONE;
                break;
            }
            case NEXTARG_OUTPUT: {
                output_file_path = argv[i];
                has_output = 1;

                next_arg = NEXTARG_NONE;
                break;
            }
            case NEXTARG_ALGORITHM: {
                if (strcmp(argv[i], "lz4") == 0) {
                    compression_algorithm = COMPRESS_LZ4;
                }

                next_arg = NEXTARG_NONE;
                break;
            }
        }
    }

    if (compress == 2) {
        printf("error: compress or decompress flag not passed. Please pass argument -c (--compress) or -d (--decompress).\n");
        return 0;
    }

    if (has_input != 1) {
        printf("error: no input file argument passed. Please pass -i (--input) and a valid input file.\n");
        return 0;
    }

    if (has_output != 1) {
        printf("error: no output file argument passed. Please pass -o (--output) and a unused name for output file.\n");
        return 0;
    }

    const char* compression_algorithm_string;

    switch (compression_algorithm) {
        case COMPRESS_LZ4: {
            compression_algorithm_string = "LZ4";
            break;
        }
        default:
        case COMPRESS_NONE: {
            compression_algorithm_string = "None";
            break;
        }
    }

    clock_t time_start = clock();

    char* input_file_buffer;
    uint64_t input_file_size = 0;

    if (compress == 1) {
        if (read_raw_file(input_file_path, &input_file_buffer, &input_file_size) != 0) {
            return 0;
        }

        FileHeader header = {{'U', 'B', 'I', 'G'}, 0, input_file_size, compression_algorithm};
        char* compressed_input_data;

        switch (compression_algorithm) {
            case COMPRESS_LZ4: {
                if (compress_lz4(input_file_buffer, &compressed_input_data, input_file_size, &header.compressed_data_size) != 0) {
                    return 0;
                }

                break;
            }
        }

        if (write_file(output_file_path, header, compressed_input_data) != 0) {
            return 0;
        }

        clock_t time_end = clock();
        double time_used = ((double) (time_end - time_start)) / CLOCKS_PER_SEC;

        // Sorry for the really long line, I think multiple printfs are slower and I could just new line for every
        // argument but I like this. You can see basically what it does here, just not how the values are caluclated.
        printf("unbig: compression successful.\nCompression Percentage: %.2f.\nSize: %llu KB -> %llu KB.\nAlgorithm Used: %s.\nTime Taken: %.3f.\n", 1 - (header.compressed_data_size / input_file_size) * 100, input_file_size / 1024, header.compressed_data_size / 1024, compression_algorithm_string, time_used);
    } else if (compress == 0) {
        FileHeader header;
        char* compressed_data;

        if (read_file(input_file_path, &header, &compressed_data) != 0) {
            return 0;
        }

        char* decompressed_output;

        switch (compression_algorithm) {
            case COMPRESS_LZ4: {
                if (decompress_lz4(compressed_data, &decompressed_output, header.compressed_data_size, header.decompressed_data_size) != 0) {
                    return 0;
                }

                break;
            }
        }

        if (write_raw_file(output_file_path, &decompressed_output, header.decompressed_data_size) != 0) {
            return 0;
        }
    }

    free(input_file_buffer);
    return 0;
}
