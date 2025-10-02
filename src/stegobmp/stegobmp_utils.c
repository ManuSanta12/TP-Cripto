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

uint32_t read_uint32_big_endian(const unsigned char *buffer) {
    uint32_t size = 0;

    size |= (uint32_t) buffer[BMP_BYTE_INDEX_0] << BMP_BYTE_SHIFT_3;
    size |= (uint32_t) buffer[BMP_BYTE_INDEX_1] << BMP_BYTE_SHIFT_2;
    size |= (uint32_t) buffer[BMP_BYTE_INDEX_2] << BMP_BYTE_SHIFT_1;
    size |= (uint32_t) buffer[BMP_BYTE_INDEX_3] << BMP_BYTE_SHIFT_0;

    return size;
}

int save_extracted_file(const unsigned char *payload_buffer, const size_t extracted_payload_size, const char * output_filename) {
    const size_t file_size = read_uint32_big_endian(payload_buffer);
    const size_t extension_start_index = BMP_INT_SIZE_BYTES + file_size;

    if (file_size == 0) {
        printf("Error: Size of extracted file is zero\n");
        return 1;
    }

    if (extension_start_index >= extracted_payload_size || payload_buffer[extension_start_index] != STEGOBMP_EXTENSION_DOT || payload_buffer[extracted_payload_size - 1] != STEGOBMP_NULL_CHARACTER) {
        printf("Error: Extracted payload does not have a valid extension (extension or null terminator missing)\n");
        return 1;
    }

    const char *extension = (const char *) &payload_buffer[extension_start_index];
    const size_t output_filename_base_lenght = strlen(output_filename);
    const size_t output_filename_extension_lenght = strlen(extension);

    char *final_output_filename = malloc(output_filename_base_lenght + output_filename_extension_lenght + STEGOBMP_NULL_CHARACTER_SIZE);
    if (!final_output_filename) {
        printf("Error: Could not allocate memory for output filename\n");
        return 1;
    }

    strcpy(final_output_filename, output_filename);
    strcat(final_output_filename, extension);

    FILE *file = fopen(final_output_filename, BMP_FILE_MODE_WRITE_BINARY);
    if (!file) {
        printf("Error: Could not open output file %s\n", final_output_filename);
        free(final_output_filename);
        return 1;
    }

    if (fwrite(payload_buffer + BMP_INT_SIZE_BYTES, BMP_BYTE_SIZE, file_size, file) != file_size) {
        printf("Error: Could not write to output file %s\n", final_output_filename);
        fclose(file);
        free(final_output_filename);
        return 1;
    }

    fclose(file);
    free(final_output_filename);
    return 0;
}