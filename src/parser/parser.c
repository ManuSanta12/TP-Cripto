#include "../../include/parser/parser.h"
#include "../../include/stegobmp/stegobmp_utils.h"

#include <string.h>
#include <stdio.h>

int parse_arguments(const int argc, char *argv[], ProgramArguments *arguments) {

    if (argc < 8) {
        printf("Usage: %s -embed -in <input> -p <bmp> -out <bmp_out> -steg <LSB1|LSB4|LSBI> [-a <aes128|aes192|aes256|3des>] [-m <ecb|cfb|ofb|cbc>] [-pass <password>]\n", argv[0]);
        printf("Usage: %s -extract -p <bmp> -out <bmp_out> -steg <LSB1|LSB4|LSBI> [-a <aes128|aes192|aes256|3des>] [-m <ecb|cfb|ofb|cbc>] [-pass <password>]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-embed") == 0) {
            arguments->embed = 1;
        } else if (strcmp(argv[i], "-extract") == 0) {
            arguments->extract = 1;
        } else if (strcmp(argv[i], "-in") == 0) {
            if (i + 1 < argc) {
                arguments->input_filename = argv[i + 1];
                i++;
            } else {
                printf("Error: Missing argument for -in\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                arguments->bmp_filename = argv[i + 1];
                i++;
            } else {
                printf("Error: Missing argument for -p\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-out") == 0) {
            if (i + 1 < argc) {
                arguments->output_bmp_filename = argv[i + 1];
                i++;
            } else {
                printf("Error: Missing argument for -out\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-steg") == 0) {
            if (i + 1 < argc) {
                arguments->steganography_method = argv[i + 1];
                i++;
            } else {
                printf("Error: Missing argument for -steg\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 < argc) {
                arguments->encryption_method = argv[i + 1];
                i++;
            } else {
                printf("Error: Missing argument for -a\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 < argc) {
                arguments->encryption_mode = argv[i + 1];
                i++;
            } else {
                printf("Error: Missing argument for -m\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-pass") == 0) {
            if (i + 1 < argc) {
                arguments->password = argv[i + 1];
                i++;
            } else {
                printf("Error: Missing argument for -pass\n");
                return 1;
            }
        }
    }

    if (!arguments->bmp_filename) {
        printf("Error: Missing required argument -p\n");
        return 1;
    }

    if (arguments->steganography_method && strcmp(arguments->steganography_method, STEGOBMP_LSB1_METHOD) != 0 &&
            strcmp(arguments->steganography_method, STEGOBMP_LSB4_METHOD) != 0 && strcmp(arguments->steganography_method, STEGOBMP_LSBI_METHOD) != 0) {
        printf("Error: Unsupported steganography method %s\n", arguments->steganography_method);
        return 1;
    }

    if (arguments->embed) {
        if (!arguments->input_filename || !arguments->output_bmp_filename || !arguments->steganography_method) {
            printf("Error: Missing required arguments for embedding\n");
            return 1;
        }
    } else if (arguments->extract) {
        if (!arguments->output_bmp_filename || !arguments->steganography_method) {
            printf("Error: Missing required arguments for extraction\n");
            return 1;
        }
    } else {
        printf("Error: Missing required argument for action -embed|extract\n");
        return 1;
    }

    return 0;
}