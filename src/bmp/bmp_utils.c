#include "../../include/bmp/bmp_utils.h"

int32_t read_int32_little_endian(const unsigned char *buffer) {
    const uint32_t b0 = buffer[BMP_BYTE_INDEX_0];
    const uint32_t b1 = buffer[BMP_BYTE_INDEX_1];
    const uint32_t b2 = buffer[BMP_BYTE_INDEX_2];
    const uint32_t b3 = buffer[BMP_BYTE_INDEX_3];
    return (int32_t) (b0 << BMP_BYTE_SHIFT_0 | b1 << BMP_BYTE_SHIFT_1 | b2 << BMP_BYTE_SHIFT_2 | b3 << BMP_BYTE_SHIFT_3);
}

int16_t read_int16_little_endian(const unsigned char *buffer) {
    const uint16_t b0 = buffer[BMP_BYTE_INDEX_0];
    const uint16_t b1 = buffer[BMP_BYTE_INDEX_1];
    return (int16_t) (b0 << BMP_BYTE_SHIFT_0 | b1 << BMP_BYTE_SHIFT_1);
}

void write_int32_little_endian(unsigned char *buffer, const int32_t value) {
    buffer[BMP_BYTE_INDEX_0] = value >> BMP_BYTE_SHIFT_0 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_1] = value >> BMP_BYTE_SHIFT_1 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_2] = value >> BMP_BYTE_SHIFT_2 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_3] = value >> BMP_BYTE_SHIFT_3 & BMP_BYTE_MASK;
}

void write_int16_little_endian(unsigned char *buffer, const int16_t value) {
    buffer[BMP_BYTE_INDEX_0] = value >> BMP_BYTE_SHIFT_0 & BMP_BYTE_MASK;
    buffer[BMP_BYTE_INDEX_1] = value >> BMP_BYTE_SHIFT_1 & BMP_BYTE_MASK;
}