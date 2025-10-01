#ifndef STEGOBMP_BMP_H
#define STEGOBMP_BMP_H

#define BMP_HEADER_SIZE 54
#define BMP_BITS_PER_PIXEL 24
#define BMP_BYTES_PER_PIXEL 3
#define BMP_NO_COMPRESSION 0

typedef struct {
    unsigned char header[BMP_HEADER_SIZE];
    unsigned char *data;
    size_t data_size;
    int width;
    int height;
    int bits_per_pixel;
    int compression;
} BMP;

BMP *bmp_read(const char *filename);
int bmp_write(const char *filename, BMP *bmp);
void bmp_free(BMP *bmp);

#endif //STEGOBMP_BMP_H