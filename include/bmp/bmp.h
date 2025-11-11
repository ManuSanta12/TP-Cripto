#ifndef STEGOBMP_BMP_H
#define STEGOBMP_BMP_H

#include <stddef.h>
#include <stdint.h>

#define BMP_HEADER_SIZE 54
#define BMP_BITS_PER_PIXEL 24
#define BMP_BYTES_PER_PIXEL 3
#define BMP_NO_COMPRESSION 0

#define BMP_HEADER_WIDTH_OFFSET 18
#define BMP_HEADER_HEIGHT_OFFSET 22
#define BMP_HEADER_BITS_PER_PIXEL_OFFSET 28
#define BMP_HEADER_COMPRESSION_OFFSET 30
// Standard BMP header offset to pixel array (bfOffBits)
#define BMP_HEADER_PIXEL_DATA_OFFSET 10

typedef struct
{
    unsigned char header[BMP_HEADER_SIZE];
    unsigned char *data;
    size_t data_size;
    int32_t width;
    int32_t height;
    int16_t bits_per_pixel;
    int32_t compression;
    int32_t pixel_data_offset; // offset to pixel array from file start
    int32_t row_bytes;         // bytes per row including padding
} BMP;

BMP *bmp_read(const char *bmp_filename);
int bmp_write(BMP *bmp, const char *output_bmp_filename);
void bmp_free(BMP *bmp);

#endif // STEGOBMP_BMP_H