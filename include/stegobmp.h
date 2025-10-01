#ifndef STEGOBMP_H
#define STEGOBMP_H

#include "bmp.h"

#define STEGOBMP_FILE_SEEK_START 0
#define STEGOBMP_FILE_SEEK_END 0
#define STEGOBMP_EXTENSION_DOT '.'

#define STEGOBMP_BYTES_PER_BYTE 8
#define STEGOBMP_MOST_SIGNIFICANT_BIT 7

#define STEGOBMP_LSB1_MASK 0xFE
#define STEGOBMP_BIT_MASK_1 0x01

int hide_file_in_bmp(
    const char *input_filename,
    BMP *bmp,
    const char *output_bmp_filename,
    const char *steganography_method,
    const char *encryption_method,
    const char *encryption_mode,
    const char *password
    );

int extract_file_from_bmp(
    const BMP *bmp,
    const char *output_filename,
    const char *steganography_method,
    const char *encryption_method,
    const char *encryption_mode,
    const char *password
    );

// TODO: check if this is OK (point 3.3)
int stegoanalyze_bmp(
    const BMP *bmp,
    int *has_file_inside,
    char *detected_steganography_method,
    const char *output_filename
    );

#endif //STEGOBMP_H