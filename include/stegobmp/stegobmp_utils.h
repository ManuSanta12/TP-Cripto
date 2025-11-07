#ifndef STEGOBMP_STEGOBMP_UTILS_H
#define STEGOBMP_STEGOBMP_UTILS_H

#include <stddef.h>
#include <stdint.h>

#define STEGOBMP_FILE_SEEK_START 0
#define STEGOBMP_FILE_SEEK_END 0
#define STEGOBMP_EXTENSION_DOT '.'
#define STEGOBMP_NULL_CHARACTER '\0'
#define STEGOBMP_NULL_CHARACTER_SIZE 1

#define STEGOBMP_LSB1_METHOD "LSB1"
#define STEGOBMP_LSB4_METHOD "LSB4"
#define STEGOBMP_LSBI_METHOD "LSBI"

unsigned char *build_payload_buffer(const char *input_filename, size_t *payload_size, char **payload_extension);

void write_uint32_big_endian(unsigned char *buffer, uint32_t value);
uint32_t read_uint32_big_endian(const unsigned char *buffer);

int save_extracted_file(const unsigned char *payload_buffer, size_t extracted_payload_size, const char * output_filename);
int stego_payload_locate_extension(const unsigned char *payload_buffer, size_t payload_size, size_t file_size, size_t *extension_offset, size_t *extension_length);

#endif //STEGOBMP_STEGOBMP_UTILS_H
