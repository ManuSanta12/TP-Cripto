#include "../../include/stegobmp/stegobmp_utils.h"
#include "../../include/bmp/bmp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *build_payload_buffer(const char *input_filename, size_t *payload_size, char **payload_extension) {
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

void write_uint32_big_endian(unsigned char *buffer, const uint32_t value) {
    buffer[BMP_BYTE_INDEX_0] = value >> BMP_BYTE_SHIFT_3 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_1] = value >> BMP_BYTE_SHIFT_2 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_2] = value >> BMP_BYTE_SHIFT_1 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_3] = value >> BMP_BYTE_SHIFT_0 & BMP_BYTE_MASK;
}