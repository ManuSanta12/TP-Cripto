#ifndef STEGOBMP_BMP_H
#define STEGOBMP_BMP_H

#include <stddef.h>
#include <stdint.h>

#define BMP_HEADER_SIZE 54
#define BMP_BITS_PER_PIXEL 24
#define BMP_BYTES_PER_PIXEL 3
#define BMP_BYTE_SIZE sizeof(unsigned char)
#define BMP_NO_COMPRESSION 0

#define BMP_HEADER_WIDTH_OFFSET 18
#define BMP_HEADER_HEIGHT_OFFSET 22
#define BMP_HEADER_BITS_PER_PIXEL_OFFSET 28
#define BMP_HEADER_COMPRESSION_OFFSET 30

#define BMP_FILE_MODE_READ_BINARY "rb"
#define BMP_FILE_MODE_WRITE_BINARY "wb"

#define BMP_INT_SIZE_BYTES 4
#define BMP_SHORT_SIZE_BYTES 2

#define BMP_BYTE_INDEX_0 0
#define BMP_BYTE_INDEX_1 1
#define BMP_BYTE_INDEX_2 2
#define BMP_BYTE_INDEX_3 3

#define BMP_BYTE_SHIFT_0 0
#define BMP_BYTE_SHIFT_1 8
#define BMP_BYTE_SHIFT_2 16
#define BMP_BYTE_SHIFT_3 24

#define BMP_BYTE_MASK 0xFF

typedef struct {
    unsigned char header[BMP_HEADER_SIZE];
    unsigned char *data;
    size_t data_size;
    int32_t width;
    int32_t height;
    int16_t bits_per_pixel;
    int32_t compression;
} BMP;

BMP *bmp_read(const char *bmp_filename);
int bmp_write(BMP *bmp, const char *output_bmp_filename);
void bmp_free(BMP *bmp);

#endif //STEGOBMP_BMP_H