#include "../../include/stegobmp/stegobmp.h"
#include "../../include/stegobmp/stegobmp_lsb.h"
#include "../../include/stegobmp/stegobmp_utils.h"
#include "../../include/crypto/crypto.h"
#include "../../include/bmp/bmp_utils.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int string_has_value(const char *value) {
    return value && value[0] != '\0';
}

int hide_file_in_bmp(const char *input_filename, BMP *bmp, const char *output_bmp_filename, const char *steganography_method, const char *encryption_method, const char *encryption_mode, const char *password) {
    size_t payload_size;
    char *payload_extension;
    unsigned char *payload_buffer = build_payload_buffer(input_filename, &payload_size, &payload_extension);
    if (!payload_buffer) {
        printf("Error: Could not prepare buffer\n");
        return 1;
    }

    const int encryption_enabled = string_has_value(encryption_method) && string_has_value(encryption_mode) && string_has_value(password);

    if (encryption_enabled) {
        unsigned char salt[CRYPTO_SALT_SIZE];
        if (RAND_bytes(salt, CRYPTO_SALT_SIZE) != 1) {
            printf("Error: Could not generate salt for encryption\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        const int iv_length = crypto_get_iv_length(encryption_method, encryption_mode);
        if (iv_length < 0 || iv_length > CRYPTO_MAX_IV_SIZE) {
            printf("Error: Unsupported cipher or mode for IV generation\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        unsigned char iv[CRYPTO_MAX_IV_SIZE] = {0};
        if (iv_length > 0 && RAND_bytes(iv, iv_length) != 1) {
            printf("Error: Could not generate IV for encryption\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        if (payload_size > (size_t) INT_MAX) {
            printf("Error: Payload too large to encrypt\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        const int block_size = crypto_get_block_size(encryption_method, encryption_mode);
        if (block_size < 0) {
            printf("Error: Unsupported cipher or mode for block size calculation\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        const size_t cipher_buffer_capacity = payload_size + (size_t) block_size;
        unsigned char *cipher_buffer = malloc(cipher_buffer_capacity);
        if (!cipher_buffer) {
            printf("Error: Could not allocate memory for cipher buffer\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        const int cipher_length = crypto_encrypt(
            payload_buffer,
            (int) payload_size,
            encryption_method,
            encryption_mode,
            password,
            salt,
            iv_length > 0 ? iv : NULL,
            cipher_buffer
        );

        if (cipher_length < 0) {
            printf("Error: Encryption failed\n");
            free(cipher_buffer);
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        const size_t metadata_size = CRYPTO_SALT_SIZE + CRYPTO_METADATA_IV_LEN_SIZE + (size_t) iv_length + BMP_INT_SIZE_BYTES;
        const size_t encrypted_section_size = metadata_size + (size_t) cipher_length;
        const size_t final_payload_size = BMP_INT_SIZE_BYTES + encrypted_section_size + STEGOBMP_NULL_CHARACTER_SIZE;

        if (encrypted_section_size > UINT32_MAX) {
            printf("Error: Encrypted payload too large to embed\n");
            free(cipher_buffer);
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        unsigned char *encrypted_payload = malloc(final_payload_size);
        if (!encrypted_payload) {
            printf("Error: Could not allocate memory for encrypted payload\n");
            free(cipher_buffer);
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }

        write_uint32_big_endian(encrypted_payload, (uint32_t) encrypted_section_size);

        unsigned char *cursor = encrypted_payload + BMP_INT_SIZE_BYTES;
        memcpy(cursor, salt, CRYPTO_SALT_SIZE);
        cursor += CRYPTO_SALT_SIZE;
        *cursor = (unsigned char) iv_length;
        cursor += CRYPTO_METADATA_IV_LEN_SIZE;
        if (iv_length > 0) {
            memcpy(cursor, iv, (size_t) iv_length);
            cursor += iv_length;
        }
        write_uint32_big_endian(cursor, (uint32_t) cipher_length);
        cursor += BMP_INT_SIZE_BYTES;
        memcpy(cursor, cipher_buffer, (size_t) cipher_length);
        cursor += cipher_length;
        *cursor = STEGOBMP_NULL_CHARACTER;

        free(cipher_buffer);
        free(payload_buffer);

        payload_buffer = encrypted_payload;
        payload_size = final_payload_size;
    }

    if (strcmp(steganography_method, STEGOBMP_LSB1_METHOD) == 0) {
        if (lsb_1_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSB1\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSB4_METHOD) == 0) {
        if (lsb_4_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSB4\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSBI_METHOD) == 0) {
        if (lsb_i_hide(bmp, payload_buffer, payload_size)) {
            printf("Error: Could not hide payload using LSBI\n");
            free(payload_buffer);
            free(payload_extension);
            return 1;
        }
    }

    free(payload_buffer);
    free(payload_extension);
    return 0;
}

int extract_file_from_bmp(const BMP *bmp, const char *output_filename, const char *steganography_method, const char *encryption_method, const char *encryption_mode, const char *password) {
    unsigned char *payload_buffer = NULL;
    size_t extracted_payload_size = 0;

    const int encryption_enabled = string_has_value(encryption_method) && string_has_value(encryption_mode) && string_has_value(password);

    if (strcmp(steganography_method, STEGOBMP_LSB1_METHOD) == 0) {
        if (encryption_enabled) {
            payload_buffer = lsb_1_retrieve_encrypted(bmp, &extracted_payload_size);
        } else {
            payload_buffer = lsb_1_retrieve(bmp, &extracted_payload_size);
        }
        if (!payload_buffer) {
            printf("Error: Could not retrieve payload using LSB1\n");
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSB4_METHOD) == 0) {
        payload_buffer = lsb_4_retrieve(bmp, &extracted_payload_size);
        if (!payload_buffer) {
            printf("Error: Could not retrieve payload using LSB4\n");
            return 1;
        }
    } else if (strcmp(steganography_method, STEGOBMP_LSBI_METHOD) == 0) {
        payload_buffer = lsb_i_retrieve(bmp, &extracted_payload_size);
        if (!payload_buffer) {
            printf("Error: Could not retrieve payload using LSBI\n");
            return 1;
        }
    }

    unsigned char *data_to_save = payload_buffer;
    size_t data_to_save_size = extracted_payload_size;

    if (encryption_enabled) {
        if (extracted_payload_size < BMP_INT_SIZE_BYTES + 1) {
            printf("Error: Payload too small to contain encrypted data\n");
            free(payload_buffer);
            return 1;
        }

        const uint32_t cipher_length = read_uint32_big_endian(payload_buffer);
        if (cipher_length == 0 || (size_t)cipher_length > extracted_payload_size - BMP_INT_SIZE_BYTES) {
            printf("Error: Encrypted payload size inconsistent\n");
            free(payload_buffer);
            return 1;
        }

        const unsigned char *ciphertext = payload_buffer + BMP_INT_SIZE_BYTES;

        const int block_size = crypto_get_block_size(encryption_method, encryption_mode);
        if (block_size < 0) {
            printf("Error: Unsupported cipher or mode for decryption\n");
            free(payload_buffer);
            return 1;
        }

        unsigned char *decrypted_buffer = malloc((size_t)cipher_length + (size_t)block_size);
        if (!decrypted_buffer) {
            printf("Error: Could not allocate memory for decrypted payload\n");
            free(payload_buffer);
            return 1;
        }

        unsigned char fixed_salt[CRYPTO_SALT_SIZE] = {0};
        unsigned char iv[CRYPTO_MAX_IV_SIZE] = {0};

        const int plain_length = crypto_decrypt(
            ciphertext,
            (int)cipher_length,
            encryption_method,
            encryption_mode,
            password,
            fixed_salt,
            NULL,
            decrypted_buffer
        );

        if (plain_length < 0) {
            printf("Error: Decryption failed\n");
            free(decrypted_buffer);
            free(payload_buffer);
            return 1;
        }

        data_to_save = decrypted_buffer;
        data_to_save_size = (size_t)plain_length;
        free(payload_buffer);
    }

    if (save_extracted_file(data_to_save, data_to_save_size, output_filename) == 1) {
        printf("Error: Could not save extracted file\n");
        if (encryption_enabled) {
            free(data_to_save);
        } else {
            free(payload_buffer);
        }
        return 1;
    }

    if (encryption_enabled) {
        free(data_to_save);
    } else {
        free(payload_buffer);
    }
    return 0;
}
