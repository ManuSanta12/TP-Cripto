#include "../../include/stegobmp/stegobmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/stegobmp/stegobmp_hide.h"
#include "../../include/stegobmp/stegobmp_utils.h"

int hide_file_in_bmp(const char *input_filename, BMP *bmp, const char *output_bmp_filename, const char *steganography_method, const char *encryption_method, const char *encryption_mode, const char *password) {
    size_t payload_size;
    char *payload_extension;
    unsigned char *payload_buffer = build_payload_buffer(input_filename, &payload_size, &payload_extension);
    if (!payload_buffer) {
        printf("Error: Could not prepare buffer\n");
        return 1;
    }

    if (strcmp(steganography_method, STEGOBMP_LSB1_METHOD) == 0) {
        if (lsb1_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSB1\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSB4_METHOD) == 0) {
        if (lsb4_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSB4\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSBI_METHOD) == 0) {
        if (lsbI_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSBI\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    }

    free(payload_buffer);
    free(payload_extension);
    return 0;
}