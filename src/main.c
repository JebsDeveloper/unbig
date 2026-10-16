#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "compress.h"
#include "file.h"

// Next argument.
enum {
    NEXTARG_NONE = 0,
    NEXTARG_ALGORITHM = 1,
    NEXTARG_INPUT = 2,
    NEXTARG_OUTPUT = 3,
    NEXTARG_MEMORY = 4
};

// Compress/decompress/undefined.
enum {
    COMPRESSARG_UNDEFINED = 0,
    COMPRESSARG_COMPRESS = 1,
    COMPRESSARG_DECOMPRESS = 2,
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
        printf("Usage:\n"
               "    unbig <args>\n"
               "Args:\n"
               "    unbig {-h, --help} Displays this help menu\n"
               "    unbig {-a, --algorithm} <alg> Choses which compression algorithm to use.\n"
               "          Algorithms:\n"
               "              LZ4: {-lz4}\n"
               "    unbig {-c, --compress} Choses to compress input to output.\n"
               "    unbig {-d, --decompress} Choses to decompress input to output.\n"
               "    unbig {-i, --input} <file> The file to use as input.\n"
               "    unbig {-o, --output} <file> The file to use as output.\n");
        
        return 0;
    }

    // Plan B: will not be replaced by bitpacking. I tried it and it works but it increases
    // speed by basically nothing and only decreases memory usage by 3 bytes.
    uint8_t compress = COMPRESSARG_UNDEFINED;
    uint8_t next_arg = NEXTARG_NONE;

    uint8_t has_input = 0;
    uint8_t has_output = 0;

    uint64_t max_memory = 0;

    const char* input_file_path;
    const char* output_file_path;

    uint8_t compression_algorithm = COMPRESS_NONE;

    // Parse the program arguments.
    for (uint8_t i = 1; i < argument_count; ++i) {
        // If bit 0 and bit 1 are both set off, no argument is next, check for a flag.
        if (next_arg == NEXTARG_NONE) {
            if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--algorithm") == 0) {
                next_arg = NEXTARG_ALGORITHM;
            } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
                next_arg = NEXTARG_INPUT;
            } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
                next_arg = NEXTARG_OUTPUT;
            } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--max-memory") == 0) {
                next_arg = NEXTARG_MEMORY;
            } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compress") == 0) {
                compress = COMPRESSARG_COMPRESS;
            } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decompress") == 0) {
                compress = COMPRESSARG_DECOMPRESS;
            }

            continue;
        }

        switch (next_arg) {
            case NEXTARG_ALGORITHM: {
                if (strcmp(argv[i], "lz4") == 0) {
                    compression_algorithm = COMPRESS_LZ4;
                }

                next_arg = NEXTARG_NONE;
                break;
            }
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
            case NEXTARG_MEMORY: {
                char* endptr;
                errno = 0;

                // Base 10 (3rd parameter) indicates decimal (normal integers, not hex or anything else).
                unsigned long long max_memory_ull = strtoull(argv[i], &endptr, 10);

                if (endptr == argv[i]) {
                    printf("error: no digits found in '%s' after flag {-m, --max-memory}", argv[i]);
                    return 0;
                } else if (errno == ERANGE) {
                    printf("error: value provided as flag {-m, --max-memory} is out of range of ull");
                    return 0;
                }

                max_memory = (uint64_t) max_memory_ull;
                break;
            }
        }
    }

    // If the compress/decompress flag is still undefined, ther user didn't define
    // whether to use compression or decompression on the files.
    if (compress == COMPRESSARG_UNDEFINED) {
        printf("error: compress or decompress flag not passed, please pass argument -c (--compress) or -d (--decompress)\n");
        return 0;
    }

    if (compression_algorithm == COMPRESS_NONE && compress != COMPRESSARG_DECOMPRESS) {
        printf("error: except for when decompressing, you must pass the algorithm to use\n");
        return 0;
    }

    // No input file was defined by the user.
    if (has_input != 1) {
        printf("error: no input file argument passed, please pass -i (--input) and a valid input file\n");
        return 0;
    }

    // No output file was defined by user.
    if (has_output != 1) {
        printf("error: no output file argument passed, please pass -o (--output) and a unused name for output file\n");
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

    if (compress == COMPRESSARG_COMPRESS) {
        Buffer input_file = {};

        if (read_raw_file(input_file_path, &input_file) != 0) {
            return 0;
        }

        FileHeader header = {{'U', 'B', 'I', 'G'}, 0, input_file.size, compression_algorithm};
        Buffer compressed_data = {};

        switch (compression_algorithm) {
            case COMPRESS_LZ4: {
                if (compress_lz4(&input_file, &compressed_data) != 0) {
                    return 0;
                }

                break;
            }
        }

        header.compressed_data_size = compressed_data.size;

        if (write_file(output_file_path, header, compressed_data.buffer) != 0) {
            free(compressed_data.buffer);
            free(input_file.buffer);

            return 0;
        }

        clock_t time_end = clock();
        double time_used = ((double) (time_end - time_start)) / CLOCKS_PER_SEC;

        printf("unbig: compression successful.\n"
               "Compression Percentage: %.2f%%.\n"
               "Size: %.2f KB -> %.2f KB.\n"
               "Algorithm Used: %s.\n"
               "Time Taken: %.3f.\n"
               "Input File: '%s'.\n"
               "Output File: '%s'.\n",
               (1 - (float) header.compressed_data_size / (float) input_file.size) * 100,
               (float) input_file.size / 1024,
               (float) header.compressed_data_size / 1024,
               compression_algorithm_string,
               time_used, 
               input_file_path, 
               output_file_path);

        free(input_file.buffer);
        free(compressed_data.buffer);
    } else if (compress == COMPRESSARG_DECOMPRESS) {
        FileHeader header;
        Buffer compressed_data = {};

        if (read_file(input_file_path, &header, &compressed_data.buffer) != 0) {
            return 0;
        }

        compressed_data.size = header.compressed_data_size;

        if (compression_algorithm == COMPRESS_NONE) {
            compression_algorithm = header.compression_algorithm;
            
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
        }

        Buffer decompressed_output = {};
        decompressed_output.size = header.decompressed_data_size;

        switch (compression_algorithm) {
            case COMPRESS_LZ4: {
                if (decompress_lz4(&compressed_data, &decompressed_output) != 0) {
                    free(compressed_data.buffer);
                    return 0;
                }

                break;
            }
        }

        if (write_raw_file(output_file_path, &decompressed_output) != 0) {
            free(decompressed_output.buffer);
            free(compressed_data.buffer);

            return 0;
        }

        clock_t time_end = clock();
        double time_used = ((double) (time_end - time_start)) / CLOCKS_PER_SEC;

        printf("unbig: decompression successful.\n"
            "Size: %.2f KB.\n"
            "Algorithm Used: %s.\n"
            "Time Taken: %.3f.\n"
            "Input File: '%s'.\n"
            "Output File: '%s'.\n",
            (float) header.decompressed_data_size / 1024,
            compression_algorithm_string,
            time_used,
            input_file_path,
            output_file_path);

        free(decompressed_output.buffer);
        free(compressed_data.buffer);
    }

    return 0;
}
