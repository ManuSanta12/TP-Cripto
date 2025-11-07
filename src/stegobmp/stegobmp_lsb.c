#include "../../include/stegobmp/stegobmp_lsb.h"
#include "../../include/stegobmp/stegobmp_utils.h"
#include "../../include/bmp/bmp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lsb_1_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
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

int lsb_4_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
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

int lsb_i_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size) {
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = payload_size * STEGOBMP_LSBI_BYTES_PER_PAYLOAD + STEGOBMP_LSBI_CONTROL_BYTES;

    if (required_amount_bytes > max_amount_bytes) {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (int i = 3; i >= 0; i--) {
        const unsigned char control_bit = STEGOBMP_LSBI_CONTROL_PATTERN >> i & STEGOBMP_LSB1_BIT_MASK_1;
        bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB1_MASK | control_bit;
        bmp_byte_index++;
    }

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++) {
        for (int bit_index = STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            const unsigned char bit = payload_buffer[payload_index] >> bit_index & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bmp_msb = bmp->data[bmp_byte_index] >> STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bit_to_hide = bit ^ bmp_msb;

            bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSBI_MASK | bit_to_hide;
            bmp_byte_index++;
        }
    }

    return 0;
}

unsigned char *lsb_1_retrieve(const BMP *bmp, size_t *extracted_payload_size) {
    const size_t bmp_data_size = bmp->data_size;
    size_t bmp_byte_index = 0;
    unsigned char size_buffer[BMP_INT_SIZE_BYTES];

    if (bmp_data_size < BMP_INT_SIZE_BYTES * STEGOBMP_LSB1_BYTES_PER_PAYLOAD) {
        printf("Error: BMP does not have enough space to extract the payload size\n");
        return NULL;
    }

    for (size_t i = 0; i < BMP_INT_SIZE_BYTES; i++) {
        unsigned char extracted_byte = 0;
        for (int bit_index = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            const unsigned char lsb = bmp->data[bmp_byte_index] & STEGOBMP_LSB1_BIT_MASK_1;
            extracted_byte |= lsb << bit_index;
            bmp_byte_index++;
        }
        size_buffer[i] = extracted_byte;
    }

    const uint32_t file_size = read_uint32_big_endian(size_buffer);

    if (file_size == 0 || file_size > bmp_data_size / STEGOBMP_LSB1_BYTES_PER_PAYLOAD - BMP_INT_SIZE_BYTES) {
        printf("Error: Extracted payload size is invalid (%u bytes)\n", file_size);
        return NULL;
    }

    const size_t min_payload_size = BMP_INT_SIZE_BYTES + file_size + STEGOBMP_NULL_CHARACTER_SIZE;
    const size_t required_bmp_bytes = min_payload_size * STEGOBMP_LSB1_BYTES_PER_PAYLOAD;

    if (required_bmp_bytes > bmp_data_size) {
        printf("Error: BMP does not have the complete payload expected (Total size expected: %zu bytes)\n", min_payload_size);
        return NULL;
    }

    const size_t max_payload_bytes = bmp_data_size / STEGOBMP_LSB1_BYTES_PER_PAYLOAD;
    unsigned char *payload_buffer = malloc(max_payload_bytes);
    if (!payload_buffer) {
        printf("Error: Could not allocate memory for payload buffer\n");
        return NULL;
    }

    memcpy(payload_buffer, size_buffer, BMP_INT_SIZE_BYTES);

    size_t payload_byte_index = BMP_INT_SIZE_BYTES;

    while (bmp_byte_index < bmp_data_size && payload_byte_index < max_payload_bytes) {
        unsigned char extracted_byte = 0;
        for (int bit_index = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            const unsigned char lsb = bmp->data[bmp_byte_index] & STEGOBMP_LSB1_BIT_MASK_1;
            extracted_byte |= lsb << bit_index;
            bmp_byte_index++;
        }

        payload_buffer[payload_byte_index] = extracted_byte;

        if (payload_byte_index >= BMP_INT_SIZE_BYTES + file_size && payload_buffer[payload_byte_index] == STEGOBMP_NULL_CHARACTER) {
            payload_byte_index++;
            break;
        }

        payload_byte_index++;
    }

    *extracted_payload_size = payload_byte_index;
    return payload_buffer;
}

unsigned char *lsb_4_retrieve(const BMP *bmp, size_t *extracted_payload_size) {
    const size_t bmp_data_size = bmp->data_size;
    size_t bmp_byte_index = 0;
    unsigned char size_buffer[BMP_INT_SIZE_BYTES];

    if (bmp_data_size < BMP_INT_SIZE_BYTES * STEGOBMP_LSB4_BYTES_PER_PAYLOAD) {
        printf("Error: BMP does not have enough space to extract the payload size\n");
        return NULL;
    }

    for (size_t i = 0; i < BMP_INT_SIZE_BYTES; i++) {
        unsigned char extracted_byte = 0;
        if (bmp_byte_index >= bmp_data_size) {
            printf("Error: BMP does not have the complete payload size\n");
            return NULL;
        }
        const unsigned char msn = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_BIT_MASK_4;
        extracted_byte |= msn << STEGOBMP_LSB4_NIBBLE_SIZE_BITS;
        bmp_byte_index++;

        if (bmp_byte_index >= bmp_data_size) {
            printf("Error: BMP does not have the complete payload size\n");
            return NULL;
        }
        const unsigned char lsn = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_BIT_MASK_4;
        extracted_byte |= lsn;
        bmp_byte_index++;

        size_buffer[i] = extracted_byte;
    }

    const uint32_t file_size = read_uint32_big_endian(size_buffer);
    const size_t min_payload_bytes = BMP_INT_SIZE_BYTES + file_size + STEGOBMP_NULL_CHARACTER_SIZE;
    const size_t required_bmp_bytes = min_payload_bytes * STEGOBMP_LSB4_BYTES_PER_PAYLOAD;

    if (file_size == 0 || file_size > (bmp_data_size / STEGOBMP_LSB4_BYTES_PER_PAYLOAD) - BMP_INT_SIZE_BYTES) {
        printf("Error: Extracted payload size is invalid (%u bytes)\n", file_size);
        return NULL;
    }

    if (required_bmp_bytes > bmp_data_size) {
        printf("Error: BMP does not have the complete payload size (Total size expected: %zu bytes)\n", min_payload_bytes);
        return NULL;
    }

    const size_t max_payload_bytes = bmp_data_size / STEGOBMP_LSB4_BYTES_PER_PAYLOAD;
    unsigned char *payload_buffer = malloc(max_payload_bytes);
    if (!payload_buffer) {
        printf("Error: Could not allocate memory for payload buffer\n");
        return NULL;
    }

    memcpy(payload_buffer, size_buffer, BMP_INT_SIZE_BYTES);

    size_t payload_byte_index = BMP_INT_SIZE_BYTES;

    while (bmp_byte_index < bmp_data_size && payload_byte_index < max_payload_bytes) {
        unsigned char extracted_byte = 0;
        if (bmp_byte_index >= bmp_data_size) {
            printf("Error: BMP does not have the complete payload expected\n");
            free(payload_buffer);
            return NULL;
        }
         const unsigned char msn = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_BIT_MASK_4;
        extracted_byte |= msn << STEGOBMP_LSB4_NIBBLE_SIZE_BITS;
        bmp_byte_index++;

        if (bmp_byte_index >= bmp_data_size) {
            printf("Error: BMP does not have the complete payload size\n");
            free(payload_buffer);
            return NULL;
        }
         const unsigned char lsn = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_BIT_MASK_4;
        extracted_byte |= lsn;
        bmp_byte_index++;

        payload_buffer[payload_byte_index] = extracted_byte;

        if (payload_byte_index >= BMP_INT_SIZE_BYTES + file_size && payload_buffer[payload_byte_index] == STEGOBMP_NULL_CHARACTER) {
            payload_byte_index++;
            break;
        }

        payload_byte_index++;
    }

    if (payload_byte_index < min_payload_bytes) {
        printf("Error: Extracted payload incomplete or null terminator missing\n");
        free(payload_buffer);
        return NULL;
    }

    *extracted_payload_size = payload_byte_index;
    return payload_buffer;
}

unsigned char *lsb_i_retrieve(const BMP *bmp, size_t *extracted_payload_size) {
    const size_t bmp_data_size = bmp->data_size;
    size_t bmp_byte_index = 0;
    unsigned char size_buffer[BMP_INT_SIZE_BYTES];
    const size_t required_control_bytes = STEGOBMP_LSBI_CONTROL_BYTES;
    const size_t required_size_bytes = BMP_INT_SIZE_BYTES * STEGOBMP_LSBI_BYTES_PER_PAYLOAD;
    const size_t min_initial_bmp_bytes = required_control_bytes + required_size_bytes;

    if (bmp_data_size < min_initial_bmp_bytes) {
        printf("Error: BMP does not have enough space to extract the LSBI control pattern and payload size\n");
        return NULL;
    }

    unsigned char extracted_control_pattern = 0;
    for (size_t i = 0; i < STEGOBMP_LSBI_CONTROL_BYTES; i++) {
        const unsigned char bit = bmp->data[bmp_byte_index] & STEGOBMP_LSBI_BIT_MASK_1;
        extracted_control_pattern |= bit << (STEGOBMP_LSBI_CONTROL_BYTES - 1 - i);
        bmp_byte_index++;
    }

    if (extracted_control_pattern != STEGOBMP_LSBI_CONTROL_PATTERN) {
        printf("Error: Extracted LSBI control pattern is invalid (Expected: 0x%X, Actual: 0x%X)\n", STEGOBMP_LSBI_CONTROL_PATTERN, extracted_control_pattern);
        return NULL;
    }

    for (size_t i = 0; i < BMP_INT_SIZE_BYTES; i++) {
        unsigned char extracted_byte = 0;
        for (int bit_index = STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            if (bmp_byte_index >= bmp_data_size) {
                printf("Error: BMP does not have the complete payload size\n");
                return NULL;
            }
            const unsigned char lsb = bmp->data[bmp_byte_index] & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bmp_msb = bmp->data[bmp_byte_index] >> STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bit = lsb ^ bmp_msb;
            extracted_byte |= bit << bit_index;
            bmp_byte_index++;
        }
        size_buffer[i] = extracted_byte;
    }

    const uint32_t file_size = read_uint32_big_endian(size_buffer);
    const size_t min_payload_bytes = BMP_INT_SIZE_BYTES + file_size + STEGOBMP_NULL_CHARACTER_SIZE;
    const size_t required_bmp_bytes = required_control_bytes + min_payload_bytes;

    if (file_size == 0) {
        printf("Error: Extracted payload size is invalid (0 bytes)\n");
        return NULL;
    }

    if (required_bmp_bytes > bmp_data_size) {
        printf("Error: BMP does not have enough space for the complete payload (Total size expected: %zu bytes)\n", min_payload_bytes);
        return NULL;
    }

    const size_t max_payload_bytes_available = (bmp_data_size - required_control_bytes) / STEGOBMP_LSBI_BYTES_PER_PAYLOAD;
    unsigned char *payload_buffer = malloc(max_payload_bytes_available);
    if (!payload_buffer) {
        printf("Error: Could not allocate memory for payload buffer\n");
        return NULL;
    }

    memcpy(payload_buffer, size_buffer, BMP_INT_SIZE_BYTES);

    size_t payload_byte_index = BMP_INT_SIZE_BYTES;

    while (bmp_byte_index + STEGOBMP_LSBI_BYTES_PER_PAYLOAD <= bmp_data_size && payload_byte_index < max_payload_bytes_available) {
        unsigned char extracted_byte = 0;
        for (int bit_index = STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--) {
            if (bmp_byte_index >= bmp_data_size) {
                printf("Error: BMP does not have the complete payload expected\n");
                free(payload_buffer);
                return NULL;
            }
            const unsigned char lsb = bmp->data[bmp_byte_index] & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bmp_msb = bmp->data[bmp_byte_index] >> STEGOBMP_LSBI_MOST_SIGNIFICANT_BIT & STEGOBMP_LSBI_BIT_MASK_1;
            const unsigned char bit = lsb ^ bmp_msb;
            extracted_byte |= bit << bit_index;
            bmp_byte_index++;
        }

        payload_buffer[payload_byte_index] = extracted_byte;

        if (payload_byte_index >= BMP_INT_SIZE_BYTES + file_size && payload_buffer[payload_byte_index] == STEGOBMP_NULL_CHARACTER) {
            payload_byte_index++;
            break;
        }

        payload_byte_index++;
    }

    if (payload_byte_index < min_payload_bytes) {
        printf("Error: Extracted payload incomplete or null terminator missing\n");
        free(payload_buffer);
        return NULL;
    }

    *extracted_payload_size = payload_byte_index;
    return payload_buffer;
}
