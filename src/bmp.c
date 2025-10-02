#include "../include/bmp.h"

#include <stdio.h>
#include <stdlib.h>

static int32_t read_int32_little_endian(const unsigned char *buffer);
static int16_t read_int16_little_endian(const unsigned char *buffer);
static void write_int32_little_endian(unsigned char *buffer, int32_t value);
static void write_int16_little_endian(unsigned char *buffer, int16_t value);

BMP *bmp_read(const char *bmp_filename) {
    FILE *file = fopen(bmp_filename, BMP_FILE_MODE_READ_BINARY);
    if (!file) {
        printf("Error: Can not open BMP file %s\n", bmp_filename);
        return NULL;
    }

    BMP *bmp = malloc(sizeof(BMP));
    if (!bmp) {
        fclose(file);
        printf("Error: Can not allocate memory for BMP\n");
        return NULL;
    }

    if (fread(bmp->header, BMP_BYTE_SIZE, BMP_HEADER_SIZE, file) != BMP_HEADER_SIZE) {
        printf("Error: Can not read BMP header\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    bmp->width = read_int32_little_endian(bmp->header + BMP_HEADER_WIDTH_OFFSET);
    bmp->height = read_int32_little_endian(bmp->header + BMP_HEADER_HEIGHT_OFFSET);
    bmp->bits_per_pixel = read_int16_little_endian(bmp->header + BMP_HEADER_BITS_PER_PIXEL_OFFSET);
    bmp->compression = read_int32_little_endian(bmp->header + BMP_HEADER_COMPRESSION_OFFSET);

    if (bmp->bits_per_pixel != BMP_BITS_PER_PIXEL || bmp->compression != BMP_NO_COMPRESSION) {
        printf("Error: Unsupported BMP format\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    bmp->data_size = bmp->width * bmp->height * BMP_BYTES_PER_PIXEL;
    bmp->data = malloc(bmp->data_size);
    if (!bmp->data) {
        printf("Error: Can not allocate memory for BMP data\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    if (fread(bmp->data, BMP_BYTE_SIZE, bmp->data_size, file) != bmp->data_size) {
        printf("Error: Can not read BMP pixel data\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    fclose(file);
    return bmp;
}

int bmp_write(BMP *bmp, const char *output_bmp_filename) {
    if (!output_bmp_filename || !bmp) {
        printf("Error: Can not open BMP\n");
        return 1;
    }

    FILE *file = fopen(output_bmp_filename, BMP_FILE_MODE_WRITE_BINARY);
    if (!file) {
        printf("Error: Can not open file for writing: %s\n", output_bmp_filename);
        return 1;
    }

    write_int32_little_endian(bmp->header + BMP_HEADER_WIDTH_OFFSET, bmp->width);
    write_int32_little_endian(bmp->header + BMP_HEADER_HEIGHT_OFFSET, bmp->height);
    write_int16_little_endian(bmp->header + BMP_HEADER_BITS_PER_PIXEL_OFFSET, bmp->bits_per_pixel);
    write_int32_little_endian(bmp->header + BMP_HEADER_COMPRESSION_OFFSET, bmp->compression);

    if (fwrite(bmp->header, BMP_BYTE_SIZE, BMP_HEADER_SIZE, file) != BMP_HEADER_SIZE) {
        printf("Error: Can not write BMP header\n");
        fclose(file);
        return 1;
    }

    if (fwrite(bmp->data, BMP_BYTE_SIZE, bmp->data_size, file) != bmp->data_size) {
        printf("Error: Can not write BMP pixel data\n");
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

void bmp_free(BMP *bmp) {
    if (!bmp)
        return;
    if (bmp->data)
        free(bmp->data);
    free(bmp);
}

static int32_t read_int32_little_endian(const unsigned char *buffer) {
    const uint32_t b0 = buffer[BMP_BYTE_INDEX_0];
    const uint32_t b1 = buffer[BMP_BYTE_INDEX_1];
    const uint32_t b2 = buffer[BMP_BYTE_INDEX_2];
    const uint32_t b3 = buffer[BMP_BYTE_INDEX_3];
    return (int32_t) (b0 << BMP_BYTE_SHIFT_0 | b1 << BMP_BYTE_SHIFT_1 | b2 << BMP_BYTE_SHIFT_2 | b3 << BMP_BYTE_SHIFT_3);
}

static int16_t read_int16_little_endian(const unsigned char *buffer) {
    const uint16_t b0 = buffer[BMP_BYTE_INDEX_0];
    const uint16_t b1 = buffer[BMP_BYTE_INDEX_1];
    return (int16_t) (b0 << BMP_BYTE_SHIFT_0 | b1 << BMP_BYTE_SHIFT_1);
}

static void write_int32_little_endian(unsigned char *buffer, const int32_t value) {
    buffer[BMP_BYTE_INDEX_0] = value >> BMP_BYTE_SHIFT_0 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_1] = value >> BMP_BYTE_SHIFT_1 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_2] = value >> BMP_BYTE_SHIFT_2 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_3] = value >> BMP_BYTE_SHIFT_3 & BMP_BYTE_MASK;
}

static void write_int16_little_endian(unsigned char *buffer, const int16_t value) {
    buffer[BMP_BYTE_INDEX_0] = value >> BMP_BYTE_SHIFT_0 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_1] = value >> BMP_BYTE_SHIFT_1 & BMP_BYTE_MASK;
}