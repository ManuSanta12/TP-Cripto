#include "../../include/stegobmp/stegobmp_utils.h"
#include "../../include/bmp/bmp_utils.h"

#include <ctype.h>
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
    size_t extension_start_index = 0;
    size_t extension_length = 0;

    if (file_size == 0) {
        printf("Error: Size of extracted file is zero\n");
        return 1;
    }

    if (!stego_payload_locate_extension(payload_buffer, extracted_payload_size, file_size, &extension_start_index, &extension_length)) {
        printf("Error: Extracted payload does not have a valid extension (extension or null terminator missing)\n");
        return 1;
    }

    const size_t output_filename_base_length = strlen(output_filename);
    char *extension = malloc(extension_length + STEGOBMP_NULL_CHARACTER_SIZE);
    if (!extension) {
        printf("Error: Could not allocate memory for extension buffer\n");
        return 1;
    }
    memcpy(extension, payload_buffer + extension_start_index, extension_length);
    extension[extension_length] = STEGOBMP_NULL_CHARACTER;

    const size_t output_filename_extension_length = strlen(extension);

    char *final_output_filename = malloc(output_filename_base_length + output_filename_extension_length + STEGOBMP_NULL_CHARACTER_SIZE);
    if (!final_output_filename) {
        printf("Error: Could not allocate memory for output filename\n");
        free(extension);
        return 1;
    }

    strcpy(final_output_filename, output_filename);
    strcat(final_output_filename, extension);

    FILE *file = fopen(final_output_filename, BMP_FILE_MODE_WRITE_BINARY);
    if (!file) {
        printf("Error: Could not open output file %s\n", final_output_filename);
        free(extension);
        free(final_output_filename);
        return 1;
    }

    if (fwrite(payload_buffer + BMP_INT_SIZE_BYTES, BMP_BYTE_SIZE, file_size, file) != file_size) {
        printf("Error: Could not write to output file %s\n", final_output_filename);
        fclose(file);
        free(extension);
        free(final_output_filename);
        return 1;
    }

    fclose(file);
    free(extension);
    free(final_output_filename);
    return 0;
}

int stego_payload_locate_extension(const unsigned char *payload_buffer, const size_t payload_size, const size_t file_size, size_t *extension_offset, size_t *extension_length) {
    const size_t start_index = BMP_INT_SIZE_BYTES + file_size;

    if (!payload_buffer || payload_size <= start_index || payload_buffer[start_index] != STEGOBMP_EXTENSION_DOT) {
        return 0;
    }

    size_t current_index = start_index + 1;
    while (current_index < payload_size) {
        const unsigned char current_char = payload_buffer[current_index];
        if (current_char == STEGOBMP_NULL_CHARACTER) {
            if (current_index == start_index + 1) {
                return 0;
            }
            if (extension_offset) {
                *extension_offset = start_index;
            }
            if (extension_length) {
                *extension_length = current_index - start_index;
            }
            return 1;
        }
        if (!isalnum(current_char) && current_char != STEGOBMP_EXTENSION_DOT) {
            return 0;
        }
        current_index++;
    }

    return 0;
}
