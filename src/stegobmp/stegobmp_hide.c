#include "../../include/stegobmp/stegobmp_hide.h"

#include <stdio.h>

int lsb1_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = payload_size * STEGOBMP_LSB1_BYTES_PER_PAYLOAD;

    if (required_amount_bytes > max_amount_bytes) {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++) {
        for (int bit_index = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            const unsigned char bit = payload_buffer[payload_index] >> bit_index & STEGOBMP_LSB1_BIT_MASK_1;
            bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB1_MASK | bit;
            bmp_byte_index++;
        }
    }

    return 0;
}

int lsb4_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = payload_size * STEGOBMP_LSB4_BYTES_PER_PAYLOAD;

    if (required_amount_bytes > max_amount_bytes) {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++) {
        const unsigned char payload_high_nibble = payload_buffer[payload_index] >> STEGOBMP_LSB4_NIBBLE_SIZE_BITS & STEGOBMP_LSB4_BIT_MASK_4;
        const unsigned char payload_low_nibble = payload_buffer[payload_index] & STEGOBMP_LSB4_BIT_MASK_4;

        bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_MASK | payload_high_nibble;
        bmp_byte_index++;

        bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_MASK | payload_low_nibble;
        bmp_byte_index++;
    }

    return 0;
}

int lsbI_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = (payload_size * STEGOBMP_LSBI_BYTES_PER_PAYLOAD) + STEGOBMP_LSBI_CONTROL_BYTES;

    if (required_amount_bytes > max_amount_bytes) {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (int i = 3; i >= 0; i--) {
        const unsigned char control_bit = (STEGOBMP_LSBI_CONTROL_PATTERN >> i) & STEGOBMP_LSB1_BIT_MASK_1;
        bmp->data[bmp_byte_index] = (bmp->data[bmp_byte_index] & STEGOBMP_LSB1_MASK) | control_bit;
        bmp_byte_index++;
    }

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++) {
        for (int bit_index = STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            const unsigned char bit = payload_buffer[payload_index] >> bit_index & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bmp_msb = (bmp->data[bmp_byte_index] >> STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT) & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bit_to_hide = bit ^ bmp_msb;

            bmp->data[bmp_byte_index] = (bmp->data[bmp_byte_index] & STEGOBMP_LSBI_MASK) | bit_to_hide;
            bmp_byte_index++;
        }
    }

    return 0;
}