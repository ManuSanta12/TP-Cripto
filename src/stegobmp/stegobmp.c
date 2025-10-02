#include "../../include/stegobmp/stegobmp.h"
#include "../../include/stegobmp/stegobmp_lsb.h"
#include "../../include/stegobmp/stegobmp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hide_file_in_bmp(const char *input_filename, BMP *bmp, const char *output_bmp_filename, const char *steganography_method, const char *encryption_method, const char *encryption_mode, const char *password) {
    size_t payload_size;
    char *payload_extension;
    unsigned char *payload_buffer = build_payload_buffer(input_filename, &payload_size, &payload_extension);
    if (!payload_buffer) {
        printf("Error: Could not prepare buffer\n");
        return 1;
    }

    if (strcmp(steganography_method, STEGOBMP_LSB1_METHOD) == 0) {
        if (lsb_1_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSB1\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSB4_METHOD) == 0) {
        if (lsb_4_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSB4\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSBI_METHOD) == 0) {
        if (lsb_i_hide(bmp, payload_buffer, payload_size)) {
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

int extract_file_from_bmp(const BMP *bmp, const char *output_filename, const char *steganography_method, const char *encryption_method, const char *encryption_mode, const char *password) {
    unsigned char *payload_buffer = NULL;
    size_t extracted_payload_size = 0;

    if (strcmp(steganography_method, STEGOBMP_LSB1_METHOD) == 0) {
        payload_buffer = lsb_1_retrieve(bmp, &extracted_payload_size);
        if (!payload_buffer) {
            printf("Error: Could not retrieve payload using LSB1\n");
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSB4_METHOD) == 0) {
        payload_buffer = lsb_4_retrieve(bmp, &extracted_payload_size);
        if (!payload_buffer) {
            printf("Error: Could not retrieve payload using LSB4\n");
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSBI_METHOD) == 0) {
        payload_buffer = lsb_i_retrieve(bmp, &extracted_payload_size);
        if (!payload_buffer) {
            printf("Error: Could not retrieve payload using LSBI\n");
            return 1;
        }
    }

    if (save_extracted_file(payload_buffer, extracted_payload_size, output_filename) == 1) {
        printf("Error: Could not save extracted file\n");
        free(payload_buffer);
        return 1;
    }

    free(payload_buffer);
    return 0;
}