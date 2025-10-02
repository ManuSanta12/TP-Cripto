#ifndef STEGOBMP_STEGOBMP_UTILS_H
#define STEGOBMP_STEGOBMP_UTILS_H

#include <stddef.h>
#include <stdint.h>

#define STEGOBMP_FILE_SEEK_START 0
#define STEGOBMP_FILE_SEEK_END 0
#define STEGOBMP_EXTENSION_DOT '.'
#define STEGOBMP_NULL_CHARACTER '\0'
#define STEGOBMP_NULL_CHARACTER_SIZE 1

unsigned char *build_payload_buffer(const char *input_filename, size_t *payload_size, char **payload_extension);
void write_uint32_big_endian(unsigned char *buffer, uint32_t value);

#endif //STEGOBMP_STEGOBMP_UTILS_H