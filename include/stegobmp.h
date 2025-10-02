#ifndef STEGOBMP_H
#define STEGOBMP_H

#include "bmp.h"

#define STEGOBMP_FILE_SEEK_START 0
#define STEGOBMP_FILE_SEEK_END 0
#define STEGOBMP_EXTENSION_DOT '.'
#define STEGOBMP_NULL_CHARACTER '\0'
#define STEGOBMP_NULL_CHARACTER_SIZE 1

#define STEGOBMP_LSB1_BYTES_PER_PAYLOAD 8
#define STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT 7
#define STEGOBMP_LSB1_MASK 0xFE
#define STEGOBMP_LSB1_BIT_MASK_1 0x01

#define STEGOBMP_LSB4_BYTES_PER_PAYLOAD 2
#define STEGOBMP_LSB4_NIBBLE_SIZE_BITS 4
#define STEGOBMP_LSB4_MASK 0xF0
#define STEGOBMP_LSB4_BIT_MASK_4 0x0F

#define STEGOBMP_LSB1_METHOD "LSB1"
#define STEGOBMP_LSB4_METHOD "LSB4"
#define STEGOBMP_LSBI_METHOD "LSBI"

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