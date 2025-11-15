#include "../../include/parser/parser.h"
#include "../../include/stegobmp/stegobmp_utils.h"

#include <string.h>
#include <stdio.h>

static void print_usage(const char *program_name) {
    printf("Usage: %s -embed -in <input> -p <bmp> -out <bmp_out> -steg <LSB1|LSB4|LSBI> [-a <aes128|aes192|aes256|3des>] [-m <ecb|cfb|ofb|cbc>] [-pass <password>]\n", program_name);
    printf("Usage: %s -extract -p <bmp> -out <file_out> -steg <LSB1|LSB4|LSBI> [-a <aes128|aes192|aes256|3des>] [-m <ecb|cfb|ofb|cbc>] [-pass <password>]\n", program_name);
    printf("Usage: %s -analyze -p <bmp> -out <file_out>\n", program_name);
}

int parse_arguments(const int argc, char *argv[], ProgramArguments *arguments) {

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-embed") == 0) {
            arguments->embed = 1;
        } else if (strcmp(argv[i], "-extract") == 0) {
            arguments->extract = 1;
        } else if (strcmp(argv[i], "-analyze") == 0) {
            arguments->analyze = 1;
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

    const int actions_selected = arguments->embed + arguments->extract + arguments->analyze;
    if (actions_selected == 0) {
        printf("Error: Missing required action (-embed | -extract | -analyze)\n");
        print_usage(argv[0]);
        return 1;
    }
    if (actions_selected > 1) {
        printf("Error: Only one action can be selected at a time\n");
        return 1;
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
    } else if (arguments->analyze) {
        if (!arguments->output_bmp_filename) {
            printf("Error: Missing required argument -out for analysis\n");
            return 1;
        }
    } else {
        printf("Error: Missing required argument for action -embed|-extract|-analyze\n");
        return 1;
    }

    int encryption_method_provided = arguments->encryption_method && arguments->encryption_method[0] != '\0';
    int encryption_mode_provided = arguments->encryption_mode && arguments->encryption_mode[0] != '\0';
    const int password_provided = arguments->password && arguments->password[0] != '\0';

    /* Defaults when password is present:
     *  - method + password, no mode    => mode = "cbc"
     *  - mode + password, no method    => method = "aes128"
     *  - only password                 => method = "aes128", mode = "cbc"
     */
    if (password_provided) {
        if (encryption_method_provided && !encryption_mode_provided) {
            arguments->encryption_mode = "cbc";
            encryption_mode_provided = 1;
        } else if (!encryption_method_provided && encryption_mode_provided) {
            arguments->encryption_method = "aes128";
            encryption_method_provided = 1;
        } else if (!encryption_method_provided && !encryption_mode_provided) {
            arguments->encryption_method = "aes128";
            arguments->encryption_mode = "cbc";
            encryption_method_provided = 1;
            encryption_mode_provided = 1;
        }
    }

    /* Without password, requiring both -a and -m avoids ambiguous configs. */
    if ((encryption_method_provided && !encryption_mode_provided) ||
        (!encryption_method_provided && encryption_mode_provided)) {
        printf("Error: Encryption requires both -a <method> and -m <mode> (or rely on defaults by providing -pass)\n");
        return 1;
    }

    return 0;
}
