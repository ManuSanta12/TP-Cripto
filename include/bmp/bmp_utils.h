#ifndef STEGOBMP_BMP_UTILS_H
#define STEGOBMP_BMP_UTILS_H

#include <stdint.h>

#define BMP_BYTE_INDEX_0 0
#define BMP_BYTE_INDEX_1 1
#define BMP_BYTE_INDEX_2 2
#define BMP_BYTE_INDEX_3 3

#define BMP_BYTE_SHIFT_0 0
#define BMP_BYTE_SHIFT_1 8
#define BMP_BYTE_SHIFT_2 16
#define BMP_BYTE_SHIFT_3 24

#define BMP_BYTE_MASK 0xFF

#define BMP_INT_SIZE_BYTES 4
#define BMP_SHORT_SIZE_BYTES 2

#define BMP_BYTE_SIZE sizeof(unsigned char)

#define BMP_FILE_MODE_READ_BINARY "rb"
#define BMP_FILE_MODE_WRITE_BINARY "wb"

int32_t read_int32_little_endian(const unsigned char *buffer);
int16_t read_int16_little_endian(const unsigned char *buffer);
void write_int32_little_endian(unsigned char *buffer, int32_t value);
void write_int16_little_endian(unsigned char *buffer, int16_t value);

#endif //STEGOBMP_BMP_UTILS_H