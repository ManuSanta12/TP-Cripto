#include "../include/stegobmp.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/bmp.h"

static unsigned char *build_payload_buffer(const char *input_filename, size_t *payload_size, char **payload_extension);
static void write_uint32_big_endian(unsigned char *buffer, uint32_t value);
static int lsb1_hide(BMP *bmp, const unsigned char *payload_buffer, size_t payload_size);
static int lsb4_hide(BMP *bmp, const unsigned char *payload_buffer, size_t payload_size);
static int lsbI_hide(BMP *bmp, const unsigned char *payload_buffer, size_t payload_size);

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

static unsigned char *build_payload_buffer(const char *input_filename, size_t *payload_size, char **payload_extension) {
    FILE *file = fopen(input_filename, BMP_FILE_MODE_READ_BINARY);
    if (!file) {
        printf("Error: Could not open file %s\n", input_filename);
        return NULL;
    }

    fseek(file, STEGOBMP_FILE_SEEK_END, SEEK_END);
    const long size = ftell(file);
    if ((unsigned long) size > UINT32_MAX) {
        printf("Error: File %s is too large to be processed (size = %ld bytes, max = %u)\n", input_filename, size, (unsigned) UINT32_MAX);
        fclose(file);
        return NULL;
    }
    const uint32_t file_size = (uint32_t) size;
    fseek(file, STEGOBMP_FILE_SEEK_START, SEEK_SET);

    const char *dot = strrchr(input_filename, STEGOBMP_EXTENSION_DOT);
    if (!dot) {
        printf("Error: Could not find extension dot in %s\n", input_filename);
        fclose(file);
        return NULL;
    }
    *payload_extension = strdup(dot);
    const size_t extension_size = strlen(*payload_extension);

    *payload_size = BMP_INT_SIZE_BYTES + file_size + extension_size + STEGOBMP_NULL_CHARACTER_SIZE;

    unsigned char *buffer = malloc(*payload_size);
    if (!buffer) {
        printf("Error: Could not allocate memory for buffer\n");
        fclose(file);
        free(*payload_extension);
        return NULL;
    }

    write_uint32_big_endian(buffer, file_size);

    if (fread(buffer + BMP_INT_SIZE_BYTES, BMP_BYTE_SIZE, file_size, file) != file_size) {
        printf("Error: Could not read file %s\n", input_filename);
        fclose(file);
        free(buffer);
        free(*payload_extension);
        return NULL;
    }

    fclose(file);

    memcpy(buffer + BMP_INT_SIZE_BYTES + file_size, *payload_extension, extension_size);
    buffer[BMP_INT_SIZE_BYTES + file_size + extension_size] = STEGOBMP_NULL_CHARACTER;

    return buffer;
}

static void write_uint32_big_endian(unsigned char *buffer, const uint32_t value) {
    buffer[BMP_BYTE_INDEX_0] = value >> BMP_BYTE_SHIFT_3 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_1] = value >> BMP_BYTE_SHIFT_2 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_2] = value >> BMP_BYTE_SHIFT_1 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_3] = value >> BMP_BYTE_SHIFT_0 & BMP_BYTE_MASK;
}

static int lsb1_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = payload_size * STEGOBMP_LSB1_BYTES_PER_PAYLOAD;

    if (required_amount_bytes > max_amount_bytes) {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++) {
        for (int bit_index = 7; bit_index >= 0; bit_index--) {
            const unsigned char bit = payload_buffer[payload_index] >> bit_index & STEGOBMP_LSB1_BIT_MASK_1;
            bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB1_MASK | bit;
            bmp_byte_index++;
        }
    }

    return 0;
}

static int lsb4_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = payload_size * STEGOBMP_LSB4_BYTES_PER_PAYLOAD;

    if (required_amount_bytes > max_amount_bytes) {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++) {
        const unsigned char payload_high_nibble = payload_buffer[payload_index] >> STEGOBMP_LSB4_NIBBLE_SIZE_BITS & STEGOBMP_LSB4_BIT_MASK_4;
        const unsigned char payload_low_nibble = payload_buffer[payload_index] & STEGOBMP_LSB4_BIT_MASK_4;

        bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_MASK | payload_high_nibble;
        bmp_byte_index++;

        bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_MASK | payload_low_nibble;
        bmp_byte_index++;
    }

    return 0;
}

static int lsbI_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = (payload_size * STEGOBMP_LSBI_BYTES_PER_PAYLOAD) + STEGOBMP_LSBI_CONTROL_BYTES;

    if (required_amount_bytes > max_amount_bytes) {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (int i = 3; i >= 0; i--) {
        const unsigned char control_bit = (STEGOBMP_LSBI_CONTROL_PATTERN >> i) & STEGOBMP_LSB1_BIT_MASK_1;
        bmp->data[bmp_byte_index] = (bmp->data[bmp_byte_index] & STEGOBMP_LSB1_MASK) | control_bit;
        bmp_byte_index++;
    }

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++) {
        for (int bit_index = STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            const unsigned char bit = payload_buffer[payload_index] >> bit_index & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bmp_msb = (bmp->data[bmp_byte_index] >> STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT) & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bit_to_hide = bit ^ bmp_msb;

            bmp->data[bmp_byte_index] = (bmp->data[bmp_byte_index] & STEGOBMP_LSBI_MASK) | bit_to_hide;
            bmp_byte_index++;
        }
    }

    return 0;
}