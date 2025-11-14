#include "../../include/stegobmp/stegobmp_lsb.h"
#include "../../include/stegobmp/stegobmp_utils.h"
#include "../../include/bmp/bmp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lsb_1_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size)
{
    if (!bmp || !payload_buffer)
        return -1;

    const uint64_t capacity = (uint64_t)bmp->data_size / (uint64_t)STEGOBMP_LSB1_BYTES_PER_PAYLOAD;
    if ((uint64_t)payload_size > capacity)
    {
        /* not enough capacity to hide payload */
        return -1;
    }

    uint64_t bmp_byte_index = 0;
    const uint64_t bmp_data_size = (uint64_t)bmp->data_size;

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++)
    {
        for (int bit_index = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; bit_index >= 0; bit_index--)
        {
            if (bmp_byte_index >= bmp_data_size)
                return -1; /* safety */
            const unsigned char bit = (unsigned char)((payload_buffer[payload_index] >> bit_index) & STEGOBMP_LSB1_BIT_MASK_1);
            bmp->data[bmp_byte_index] = (unsigned char)((bmp->data[bmp_byte_index] & STEGOBMP_LSB1_MASK) | bit);
            bmp_byte_index++;
        }
    }

    /* leave the rest of the pixels unchanged */
    return 0;
}

int lsb_4_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size)
{
    const size_t max_amount_bytes = bmp->data_size;
    const size_t required_amount_bytes = payload_size * STEGOBMP_LSB4_BYTES_PER_PAYLOAD;

    if (required_amount_bytes > max_amount_bytes)
    {
        printf("Error: BMP does not have enough space to hide the payload\n");
        return 1;
    }

    size_t bmp_byte_index = 0;

    for (size_t payload_index = 0; payload_index < payload_size; payload_index++)
    {
        const unsigned char payload_high_nibble = payload_buffer[payload_index] >> STEGOBMP_LSB4_NIBBLE_SIZE_BITS & STEGOBMP_LSB4_BIT_MASK_4;
        const unsigned char payload_low_nibble = payload_buffer[payload_index] & STEGOBMP_LSB4_BIT_MASK_4;

        bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_MASK | payload_high_nibble;
        bmp_byte_index++;

        bmp->data[bmp_byte_index] = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_MASK | payload_low_nibble;
        bmp_byte_index++;
    }

    return 0;
}

int lsb_i_hide(BMP *bmp, const unsigned char *payload_buffer, const size_t payload_size)
{
    if (!bmp || !payload_buffer)
        return -1;

    const uint64_t total_pixel_bytes = (uint64_t)bmp->data_size;
    if (total_pixel_bytes < 4)
    {
        return -1;
    }

    /* build list of indices for payload bits: start at raw offset 4, skip red channel (idx%3==2) */
    unsigned int *msg_idx = malloc(sizeof(unsigned int) * (size_t)total_pixel_bytes);
    if (!msg_idx)
        return -1;
    size_t msg_count = 0;
    for (uint64_t i = 4; i < total_pixel_bytes; ++i)
    {
        if ((i % 3) == 2)
            continue; /* skip red channel */
        msg_idx[msg_count++] = (unsigned int)i;
    }

    const uint64_t payload_bits = (uint64_t)payload_size * 8ULL;
    if (payload_bits > msg_count)
    {
        free(msg_idx);
        return -1;
    }

    unsigned char *pixels = bmp->data; /* in-memory buffer */

    /* compute costs per pattern (bits 1..2) to decide inversion mask */
    uint64_t cost0[4] = {0, 0, 0, 0}, cost1[4] = {0, 0, 0, 0};
    for (uint64_t j = 0; j < payload_bits; ++j)
    {
        size_t idx = msg_idx[j];
        unsigned char pix = pixels[idx];
        int pattern = (pix & 0x06) >> 1;
        int orig = pix & 1;
        uint32_t byte_idx = (uint32_t)(j / 8);
        int bit_in_byte = (int)(j % 8);
        int desired = (payload_buffer[byte_idx] >> (7 - bit_in_byte)) & 1;
        if (orig != desired)
            cost0[pattern]++;
        if (orig != (desired ^ 1))
            cost1[pattern]++;
    }

    int must_change[4];
    for (int p = 0; p < 4; ++p)
        must_change[p] = (cost1[p] < cost0[p]) ? 1 : 0;

    /* prepare out buffer (copy) and write mask into first 4 raw pixel bytes (direct mapping) */
    unsigned char *out_pixels = malloc((size_t)total_pixel_bytes);
    if (!out_pixels)
    {
        free(msg_idx);
        return -1;
    }
    memcpy(out_pixels, pixels, (size_t)total_pixel_bytes);
    for (int i = 0; i < 4; ++i)
    {
        out_pixels[i] = (unsigned char)((out_pixels[i] & 0xFE) | (must_change[i] & 1));
    }

    /* write payload bits applying chosen mask per pattern */
    for (uint64_t j = 0; j < payload_bits; ++j)
    {
        size_t idx = msg_idx[j];
        unsigned char src = out_pixels[idx];
        int pattern = (src & 0x06) >> 1;
        uint32_t byte_idx = (uint32_t)(j / 8);
        int bit_in_byte = (int)(j % 8);
        int desired = (payload_buffer[byte_idx] >> (7 - bit_in_byte)) & 1;
        if (must_change[pattern])
            desired ^= 1;
        out_pixels[idx] = (unsigned char)((src & 0xFE) | desired);
    }

    /* copy modified pixels back into bmp->data */
    memcpy(bmp->data, out_pixels, (size_t)total_pixel_bytes);

    free(msg_idx);
    free(out_pixels);
    return 0;
}

unsigned char *lsb_1_retrieve(const BMP *bmp, size_t *extracted_payload_size)
{
    if (!bmp || !extracted_payload_size)
        return NULL;

    const unsigned char *data = bmp->data;
    const size_t data_size = bmp->data_size;

    const size_t max_payload_bytes = data_size / STEGOBMP_LSB1_BYTES_PER_PAYLOAD;
    if (max_payload_bytes < BMP_INT_SIZE_BYTES + STEGOBMP_NULL_CHARACTER_SIZE)
        return NULL;

    uint64_t cursor = 0;
    unsigned char size_buf[BMP_INT_SIZE_BYTES];

    for (size_t byte_index = 0; byte_index < BMP_INT_SIZE_BYTES; ++byte_index)
    {
        unsigned char acc = 0;
        for (int b = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; b >= 0; --b)
        {
            if (cursor >= data_size)
                return NULL;

            const unsigned char bit = data[cursor++] & STEGOBMP_LSB1_BIT_MASK_1;
            acc |= (unsigned char)(bit << b);
        }
        size_buf[byte_index] = acc;
    }

    const uint32_t file_size = read_uint32_big_endian(size_buf);
    if (file_size == 0)
        return NULL;

    const size_t max_possible_payload = max_payload_bytes;
    unsigned char *buffer = malloc(max_possible_payload);
    if (!buffer)
        return NULL;

    memcpy(buffer, size_buf, BMP_INT_SIZE_BYTES);

    size_t out_index = BMP_INT_SIZE_BYTES;
    int found_terminator = 0;

    while (out_index < max_possible_payload)
    {
        unsigned char acc = 0;
        for (int b = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; b >= 0; --b)
        {
            if (cursor >= data_size)
                goto end_of_data;

            const unsigned char bit = data[cursor++] & STEGOBMP_LSB1_BIT_MASK_1;
            acc |= (unsigned char)(bit << b);
        }

        buffer[out_index++] = acc;

        if (out_index > BMP_INT_SIZE_BYTES + file_size && acc == STEGOBMP_NULL_CHARACTER)
        {
            found_terminator = 1;
            break;
        }
    }

end_of_data:

    if (!found_terminator)
    {
        free(buffer);
        return NULL;
    }

    *extracted_payload_size = out_index;
    return buffer;
}

unsigned char *lsb_1_retrieve_encrypted(const BMP *bmp, size_t *extracted_payload_size)
{
    if (!bmp || !extracted_payload_size)
        return NULL;

    const unsigned char *data = bmp->data;
    const size_t data_size = bmp->data_size;

    const size_t max_payload_bytes = data_size / STEGOBMP_LSB1_BYTES_PER_PAYLOAD;
    if (max_payload_bytes < BMP_INT_SIZE_BYTES)
        return NULL;

    uint64_t cursor = 0;
    unsigned char size_buf[BMP_INT_SIZE_BYTES];

    for (size_t byte_index = 0; byte_index < BMP_INT_SIZE_BYTES; ++byte_index)
    {
        unsigned char acc = 0;
        for (int b = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; b >= 0; --b)
        {
            if (cursor >= data_size)
                return NULL;

            const unsigned char bit = data[cursor++] & STEGOBMP_LSB1_BIT_MASK_1;
            acc |= (unsigned char)(bit << b);
        }
        size_buf[byte_index] = acc;
    }

    const uint32_t cipher_size = read_uint32_big_endian(size_buf);
    if (cipher_size == 0 || cipher_size > max_payload_bytes - BMP_INT_SIZE_BYTES)
        return NULL;

    const size_t total_size = BMP_INT_SIZE_BYTES + (size_t)cipher_size;
    unsigned char *buffer = malloc(total_size);
    if (!buffer)
        return NULL;

    memcpy(buffer, size_buf, BMP_INT_SIZE_BYTES);

    size_t out_index = BMP_INT_SIZE_BYTES;
    while (out_index < total_size)
    {
        unsigned char acc = 0;
        for (int b = STEGOBMP_LSB1_MOST_SIGNIFICANT_BIT; b >= 0; --b)
        {
            if (cursor >= data_size)
            {
                free(buffer);
                return NULL;
            }

            const unsigned char bit = data[cursor++] & STEGOBMP_LSB1_BIT_MASK_1;
            acc |= (unsigned char)(bit << b);
        }
        buffer[out_index++] = acc;
    }

    *extracted_payload_size = total_size;
    return buffer;
}


unsigned char *lsb_4_retrieve(const BMP *bmp, size_t *extracted_payload_size)
{
    const size_t bmp_data_size = bmp->data_size;
    size_t bmp_byte_index = 0;
    unsigned char size_buffer[BMP_INT_SIZE_BYTES];

    if (bmp_data_size < BMP_INT_SIZE_BYTES * STEGOBMP_LSB4_BYTES_PER_PAYLOAD)
    {
        printf("Error: BMP does not have enough space to extract the payload size\n");
        return NULL;
    }

    for (size_t i = 0; i < BMP_INT_SIZE_BYTES; i++)
    {
        unsigned char extracted_byte = 0;
        if (bmp_byte_index >= bmp_data_size)
        {
            printf("Error: BMP does not have the complete payload size\n");
            return NULL;
        }
        const unsigned char msn = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_BIT_MASK_4;
        extracted_byte |= msn << STEGOBMP_LSB4_NIBBLE_SIZE_BITS;
        bmp_byte_index++;

        if (bmp_byte_index >= bmp_data_size)
        {
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

    if (file_size == 0 || file_size > (bmp_data_size / STEGOBMP_LSB4_BYTES_PER_PAYLOAD) - BMP_INT_SIZE_BYTES)
    {
        printf("Error: Extracted payload size is invalid (%u bytes)\n", file_size);
        return NULL;
    }

    if (required_bmp_bytes > bmp_data_size)
    {
        printf("Error: BMP does not have the complete payload size (Total size expected: %zu bytes)\n", min_payload_bytes);
        return NULL;
    }

    const size_t max_payload_bytes = bmp_data_size / STEGOBMP_LSB4_BYTES_PER_PAYLOAD;
    unsigned char *payload_buffer = malloc(max_payload_bytes);
    if (!payload_buffer)
    {
        printf("Error: Could not allocate memory for payload buffer\n");
        return NULL;
    }

    memcpy(payload_buffer, size_buffer, BMP_INT_SIZE_BYTES);

    size_t payload_byte_index = BMP_INT_SIZE_BYTES;

    while (bmp_byte_index < bmp_data_size && payload_byte_index < max_payload_bytes)
    {
        unsigned char extracted_byte = 0;
        if (bmp_byte_index >= bmp_data_size)
        {
            printf("Error: BMP does not have the complete payload expected\n");
            free(payload_buffer);
            return NULL;
        }
        const unsigned char msn = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_BIT_MASK_4;
        extracted_byte |= msn << STEGOBMP_LSB4_NIBBLE_SIZE_BITS;
        bmp_byte_index++;

        if (bmp_byte_index >= bmp_data_size)
        {
            printf("Error: BMP does not have the complete payload size\n");
            free(payload_buffer);
            return NULL;
        }
        const unsigned char lsn = bmp->data[bmp_byte_index] & STEGOBMP_LSB4_BIT_MASK_4;
        extracted_byte |= lsn;
        bmp_byte_index++;

        payload_buffer[payload_byte_index] = extracted_byte;

        if (payload_byte_index >= BMP_INT_SIZE_BYTES + file_size && payload_buffer[payload_byte_index] == STEGOBMP_NULL_CHARACTER)
        {
            payload_byte_index++;
            break;
        }

        payload_byte_index++;
    }

    if (payload_byte_index < min_payload_bytes)
    {
        printf("Error: Extracted payload incomplete or null terminator missing\n");
        free(payload_buffer);
        return NULL;
    }

    *extracted_payload_size = payload_byte_index;
    return payload_buffer;
}

unsigned char *lsb_i_retrieve(const BMP *bmp, size_t *extracted_payload_size)
{
    if (!bmp)
        return NULL;
    const unsigned char *data = bmp->data;
    const uint64_t data_size = (uint64_t)bmp->data_size;
    if (data_size < 4)
        return NULL;

    int control_pattern = 0;
    for (int i = 0; i < 4; ++i)
    {
        control_pattern = (control_pattern << 1) | (data[i] & 1);
    }

    if (control_pattern == STEGOBMP_LSBI_CONTROL_PATTERN)
    {
        uint64_t idx = STEGOBMP_LSBI_CONTROL_BYTES;
        unsigned char size_buf[BMP_INT_SIZE_BYTES];
        for (size_t i = 0; i < BMP_INT_SIZE_BYTES; ++i)
        {
            unsigned char acc = 0;
            for (int b = 7; b >= 0; --b)
            {
                if (idx >= data_size)
                    return NULL;
                unsigned char lsb = data[idx] & 1;
                unsigned char msb = (data[idx] >> 7) & 1;
                unsigned char bit = lsb ^ msb;
                acc |= (unsigned char)(bit << b);
                idx++;
            }
            size_buf[i] = acc;
        }
        const uint32_t file_size = read_uint32_big_endian(size_buf);
        if (file_size == 0)
            return NULL;
        const size_t min_bytes = BMP_INT_SIZE_BYTES + file_size + STEGOBMP_NULL_CHARACTER_SIZE;
        unsigned char *buffer = malloc(min_bytes + 64);
        if (!buffer)
            return NULL;
        memcpy(buffer, size_buf, BMP_INT_SIZE_BYTES);
        size_t out_index = BMP_INT_SIZE_BYTES;
        while (out_index < min_bytes + 64 && idx < data_size)
        {
            unsigned char acc = 0;
            for (int b = 7; b >= 0; --b)
            {
                if (idx >= data_size)
                    break;
                unsigned char lsb = data[idx] & 1;
                unsigned char msb = (data[idx] >> 7) & 1;
                unsigned char bit = lsb ^ msb;
                acc |= (unsigned char)(bit << b);
                idx++;
            }
            buffer[out_index] = acc;
            if (out_index >= BMP_INT_SIZE_BYTES + file_size && acc == STEGOBMP_NULL_CHARACTER)
            {
                out_index++;
                *extracted_payload_size = out_index;
                return buffer;
            }
            out_index++;
        }
        free(buffer);
        return NULL;
    }

    unsigned int *msg_idx = malloc(sizeof(unsigned int) * (size_t)data_size);
    if (!msg_idx)
        return NULL;
    size_t msg_count = 0;
    for (uint64_t i = 4; i < data_size; ++i)
    {
        if ((i % 3) == 2)
            continue;
        msg_idx[msg_count++] = (unsigned int)i;
    }
    if (msg_count < BMP_INT_SIZE_BYTES * 8)
    {
        free(msg_idx);
        return NULL;
    }
    int must_change[4];
    for (int i = 0; i < 4; ++i)
        must_change[i] = data[i] & 1;

    unsigned char size_buf[BMP_INT_SIZE_BYTES];
    uint64_t bit_index = 0;
    for (size_t i = 0; i < BMP_INT_SIZE_BYTES; ++i)
    {
        unsigned char acc = 0;
        for (int b = 0; b < 8; ++b)
        {
            if (bit_index >= msg_count)
            {
                free(msg_idx);
                return NULL;
            }
            size_t di = msg_idx[bit_index++];
            unsigned char pix = data[di];
            int bit = pix & 1;
            int pattern = (pix & 0x06) >> 1;
            if (must_change[pattern])
                bit ^= 1;
            acc = (unsigned char)((acc << 1) | (unsigned char)bit);
        }
        size_buf[i] = acc;
    }
    const uint32_t file_size = read_uint32_big_endian(size_buf);
    if (file_size == 0)
    {
        free(msg_idx);
        return NULL;
    }
    const size_t min_bytes = BMP_INT_SIZE_BYTES + file_size + STEGOBMP_NULL_CHARACTER_SIZE;
    const uint64_t needed_bits = (uint64_t)min_bytes * 8ULL;
    if (needed_bits > msg_count)
    {
        free(msg_idx);
        return NULL;
    }
    unsigned char *buffer = malloc(min_bytes + 64);
    if (!buffer)
    {
        free(msg_idx);
        return NULL;
    }
    memcpy(buffer, size_buf, BMP_INT_SIZE_BYTES);
    size_t out_index = BMP_INT_SIZE_BYTES;
    while (out_index < min_bytes + 64)
    {
        unsigned char acc = 0;
        for (int b = 0; b < 8; ++b)
        {
            if (bit_index >= msg_count)
            {
                free(msg_idx);
                free(buffer);
                return NULL;
            }
            size_t di = msg_idx[bit_index++];
            unsigned char pix = data[di];
            int bit = pix & 1;
            int pattern = (pix & 0x06) >> 1;
            if (must_change[pattern])
                bit ^= 1;
            acc = (unsigned char)((acc << 1) | (unsigned char)bit);
        }
        buffer[out_index] = acc;
        if (out_index >= BMP_INT_SIZE_BYTES + file_size && acc == STEGOBMP_NULL_CHARACTER)
        {
            out_index++;
            *extracted_payload_size = out_index;
            free(msg_idx);
            return buffer;
        }
        out_index++;
    }
    free(msg_idx);
    free(buffer);
    return NULL;
}
