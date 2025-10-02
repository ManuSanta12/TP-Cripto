#ifndef STEGOBMP_STEGOBMP_HIDE_H
#define STEGOBMP_STEGOBMP_HIDE_H

#include "../bmp/bmp.h"

#define STEGOBMP_LSB1_BYTES_PER_PAYLOAD 8
#define STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT 7
#define STEGOBMP_LSB1_MASK 0xFE
#define STEGOBMP_LSB1_BIT_MASK_1 0x01

#define STEGOBMP_LSB4_BYTES_PER_PAYLOAD 2
#define STEGOBMP_LSB4_NIBBLE_SIZE_BITS 4
#define STEGOBMP_LSB4_MASK 0xF0
#define STEGOBMP_LSB4_BIT_MASK_4 0x0F

#define STEGOBMP_LSBI_BYTES_PER_PAYLOAD 8
#define STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT 7
#define STEGOBMP_LSBI_MASK 0xFE
#define STEGOBMP_LSBI_BIT_MASK_1 0x01
#define STEGOBMP_LSBI_CONTROL_BYTES 4
#define STEGOBMP_LSBI_CONTROL_PATTERN 0xA

int lsb_1_hide(BMP *bmp, const unsigned char *payload_buffer, size_t payload_size);
int lsb_4_hide(BMP *bmp, const unsigned char *payload_buffer, size_t payload_size);
int lsb_i_hide(BMP *bmp, const unsigned char *payload_buffer, size_t payload_size);

unsigned char *lsb_1_retrieve(const BMP *bmp, size_t *extracted_payload_size);
unsigned char *lsb_4_retrieve(const BMP *bmp, size_t *extracted_payload_size);
unsigned char *lsb_i_retrieve(const BMP *bmp, size_t *extracted_payload_size);

#endif //STEGOBMP_STEGOBMP_HIDE_H