#include "../../include/bmp/bmp.h"
#include "../../include/bmp/bmp_utils.h"

#include <stdio.h>
#include <stdlib.h>

BMP *bmp_read(const char *bmp_filename)
{
    FILE *file = fopen(bmp_filename, BMP_FILE_MODE_READ_BINARY);
    if (!file)
    {
        printf("Error: Can not open BMP file %s\n", bmp_filename);
        return NULL;
    }

    BMP *bmp = malloc(sizeof(BMP));
    if (!bmp)
    {
        fclose(file);
        printf("Error: Can not allocate memory for BMP\n");
        return NULL;
    }

    if (fread(bmp->header, BMP_BYTE_SIZE, BMP_HEADER_SIZE, file) != BMP_HEADER_SIZE)
    {
        printf("Error: Can not read BMP header\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    bmp->width = read_int32_little_endian(bmp->header + BMP_HEADER_WIDTH_OFFSET);
    bmp->height = read_int32_little_endian(bmp->header + BMP_HEADER_HEIGHT_OFFSET);
    bmp->bits_per_pixel = read_int16_little_endian(bmp->header + BMP_HEADER_BITS_PER_PIXEL_OFFSET);
    bmp->compression = read_int32_little_endian(bmp->header + BMP_HEADER_COMPRESSION_OFFSET);
    bmp->pixel_data_offset = read_int32_little_endian(bmp->header + BMP_HEADER_PIXEL_DATA_OFFSET);

    if (bmp->bits_per_pixel != BMP_BITS_PER_PIXEL || bmp->compression != BMP_NO_COMPRESSION)
    {
        printf("Error: Unsupported BMP format\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    // Compute row size with padding to 4-byte boundary
    int64_t row_bytes = ((int64_t)bmp->width * BMP_BYTES_PER_PIXEL + 3) & ~3LL;
    if (row_bytes <= 0)
    {
        printf("Error: Invalid BMP dimensions\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }
    bmp->row_bytes = (int32_t)row_bytes;
    bmp->data_size = (size_t)row_bytes * (size_t)bmp->height;
    bmp->data = malloc(bmp->data_size);
    if (!bmp->data)
    {
        printf("Error: Can not allocate memory for BMP data\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    // Seek to pixel array offset and read full rows including padding
    if (fseek(file, bmp->pixel_data_offset, SEEK_SET) != 0)
    {
        printf("Error: Can not seek to BMP pixel data\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    if (fread(bmp->data, BMP_BYTE_SIZE, bmp->data_size, file) != bmp->data_size)
    {
        printf("Error: Can not read BMP pixel data\n");
        fclose(file);
        bmp_free(bmp);
        return NULL;
    }

    fclose(file);
    return bmp;
}

int bmp_write(BMP *bmp, const char *output_bmp_filename)
{
    if (!output_bmp_filename || !bmp)
    {
        printf("Error: Can not open BMP\n");
        return 1;
    }

    FILE *file = fopen(output_bmp_filename, BMP_FILE_MODE_WRITE_BINARY);
    if (!file)
    {
        printf("Error: Can not open file for writing: %s\n", output_bmp_filename);
        return 1;
    }

    write_int32_little_endian(bmp->header + BMP_HEADER_WIDTH_OFFSET, bmp->width);
    write_int32_little_endian(bmp->header + BMP_HEADER_HEIGHT_OFFSET, bmp->height);
    write_int16_little_endian(bmp->header + BMP_HEADER_BITS_PER_PIXEL_OFFSET, bmp->bits_per_pixel);
    write_int32_little_endian(bmp->header + BMP_HEADER_COMPRESSION_OFFSET, bmp->compression);

    if (fwrite(bmp->header, BMP_BYTE_SIZE, BMP_HEADER_SIZE, file) != BMP_HEADER_SIZE)
    {
        printf("Error: Can not write BMP header\n");
        fclose(file);
        return 1;
    }

    // Write pixel array. If header offset > header size, pad with zeros until offset
    long current = ftell(file);
    if (current < bmp->pixel_data_offset)
    {
        const long pad = bmp->pixel_data_offset - current;
        unsigned char zero = 0;
        for (long i = 0; i < pad; ++i)
            fwrite(&zero, 1, 1, file);
    }

    if (fwrite(bmp->data, BMP_BYTE_SIZE, bmp->data_size, file) != bmp->data_size)
    {
        printf("Error: Can not write BMP pixel data\n");
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

void bmp_free(BMP *bmp)
{
    if (!bmp)
        return;
    if (bmp->data)
        free(bmp->data);
    free(bmp);
}