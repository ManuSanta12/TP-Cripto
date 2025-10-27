#include "../../include/crypto/crypto.h"

#include <openssl/evp.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>

static int is_null_or_empty(const char *value) {
    return !value || value[0] == '\0';
}

static int passthrough_copy(unsigned char *destination, const unsigned char *source, int length) {
    if (!destination || !source || length < 0) {
        printf("Error: Invalid arguments for passthrough copy\n");
        return -1;
    }

    if (destination != source && length > 0) {
        memmove(destination, source, (size_t) length);
    }

    return length;
}

static const EVP_CIPHER *resolve_cipher(const char *method, const char *mode) {
    if (is_null_or_empty(method) || is_null_or_empty(mode)) {
        return NULL;
    }

    const int method_is_aes128 = strcasecmp(method, "aes128") == 0;
    const int method_is_aes192 = strcasecmp(method, "aes192") == 0;
    const int method_is_aes256 = strcasecmp(method, "aes256") == 0;
    const int method_is_3des = strcasecmp(method, "3des") == 0;

    if (method_is_aes128) {
        if (strcasecmp(mode, "ecb") == 0) return EVP_aes_128_ecb();
        if (strcasecmp(mode, "cbc") == 0) return EVP_aes_128_cbc();
        if (strcasecmp(mode, "cfb") == 0) return EVP_aes_128_cfb128();
        if (strcasecmp(mode, "ofb") == 0) return EVP_aes_128_ofb();
    } else if (method_is_aes192) {
        if (strcasecmp(mode, "ecb") == 0) return EVP_aes_192_ecb();
        if (strcasecmp(mode, "cbc") == 0) return EVP_aes_192_cbc();
        if (strcasecmp(mode, "cfb") == 0) return EVP_aes_192_cfb128();
        if (strcasecmp(mode, "ofb") == 0) return EVP_aes_192_ofb();
    } else if (method_is_aes256) {
        if (strcasecmp(mode, "ecb") == 0) return EVP_aes_256_ecb();
        if (strcasecmp(mode, "cbc") == 0) return EVP_aes_256_cbc();
        if (strcasecmp(mode, "cfb") == 0) return EVP_aes_256_cfb128();
        if (strcasecmp(mode, "ofb") == 0) return EVP_aes_256_ofb();
    } else if (method_is_3des) {
        if (strcasecmp(mode, "ecb") == 0) return EVP_des_ede3_ecb();
        if (strcasecmp(mode, "cbc") == 0) return EVP_des_ede3_cbc();
        if (strcasecmp(mode, "cfb") == 0) return EVP_des_ede3_cfb64();
        if (strcasecmp(mode, "ofb") == 0) return EVP_des_ede3_ofb();
    }

    return NULL;
}

static int derive_key(const EVP_CIPHER *cipher, const char *password, const unsigned char *salt, unsigned char *key_buffer) {
    if (!cipher || is_null_or_empty(password) || !key_buffer) {
        return 0;
    }

    unsigned char discard_iv[EVP_MAX_IV_LENGTH];
    const int expected_key_length = EVP_CIPHER_key_length(cipher);
    const int derived_length = EVP_BytesToKey(
        cipher,
        EVP_sha256(),
        salt,
        (const unsigned char *) password,
        (int) strlen(password),
        1,
        key_buffer,
        discard_iv
    );

    if (derived_length != expected_key_length) {
        printf("Error: Could not derive key from password\n");
        return 0;
    }

    return 1;
}

int crypto_encrypt(
    const unsigned char *plain_text,
    int plain_tex_lenght,
    const char *method,
    const char *mode,
    const char *password,
    const unsigned char *salt,
    const unsigned char *iv,
    unsigned char *ciphertext) {

    if (!plain_text || !ciphertext || plain_tex_lenght < 0) {
        printf("Error: Invalid arguments for encryption\n");
        return -1;
    }

    if (is_null_or_empty(method) || is_null_or_empty(mode) || is_null_or_empty(password)) {
        return passthrough_copy(ciphertext, plain_text, plain_tex_lenght);
    }

    const EVP_CIPHER *cipher = resolve_cipher(method, mode);
    if (!cipher) {
        printf("Error: Unsupported cipher method (%s) or mode (%s)\n", method ? method : "null", mode ? mode : "null");
        return -1;
    }

    const int iv_length = EVP_CIPHER_iv_length(cipher);
    if (iv_length > 0 && !iv) {
        printf("Error: Selected cipher mode requires an IV\n");
        return -1;
    }

    unsigned char key_buffer[EVP_MAX_KEY_LENGTH];
    if (!derive_key(cipher, password, salt, key_buffer)) {
        memset(key_buffer, 0, sizeof(key_buffer));
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        printf("Error: Could not allocate cipher context\n");
        memset(key_buffer, 0, sizeof(key_buffer));
        return -1;
    }

    int status = -1;
    int current_length = 0;
    int total_length = 0;

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL) != 1) {
        printf("Error: Could not initialise encryption operation\n");
        goto cleanup;
    }

    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key_buffer, iv_length > 0 ? iv : NULL) != 1) {
        printf("Error: Could not set key and IV for encryption\n");
        goto cleanup;
    }

    if (EVP_EncryptUpdate(ctx, ciphertext, &current_length, plain_text, plain_tex_lenght) != 1) {
        printf("Error: Could not encrypt data\n");
        goto cleanup;
    }
    total_length = current_length;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + total_length, &current_length) != 1) {
        printf("Error: Could not finalise encryption\n");
        goto cleanup;
    }
    total_length += current_length;

    status = total_length;

cleanup:
    EVP_CIPHER_CTX_free(ctx);
    memset(key_buffer, 0, sizeof(key_buffer));
    return status;
}

int crypto_get_iv_length(const char *method, const char *mode) {
    const EVP_CIPHER *cipher = resolve_cipher(method, mode);
    if (!cipher) {
        return -1;
    }
    return EVP_CIPHER_iv_length(cipher);
}

int crypto_get_block_size(const char *method, const char *mode) {
    const EVP_CIPHER *cipher = resolve_cipher(method, mode);
    if (!cipher) {
        return -1;
    }
    return EVP_CIPHER_block_size(cipher);
}

int crypto_decrypt(
    const unsigned char *ciphertext,
    int cipher_text_length,
    const char *method,
    const char *mode,
    const char *password,
    const unsigned char *salt,
    const unsigned char *iv,
    unsigned char *plain_text) {

    if (!ciphertext || !plain_text || cipher_text_length < 0) {
        printf("Error: Invalid arguments for decryption\n");
        return -1;
    }

    if (is_null_or_empty(method) || is_null_or_empty(mode) || is_null_or_empty(password)) {
        return passthrough_copy(plain_text, ciphertext, cipher_text_length);
    }

    const EVP_CIPHER *cipher = resolve_cipher(method, mode);
    if (!cipher) {
        printf("Error: Unsupported cipher method (%s) or mode (%s)\n", method ? method : "null", mode ? mode : "null");
        return -1;
    }

    const int iv_length = EVP_CIPHER_iv_length(cipher);
    if (iv_length > 0 && !iv) {
        printf("Error: Selected cipher mode requires an IV\n");
        return -1;
    }

    unsigned char key_buffer[EVP_MAX_KEY_LENGTH];
    if (!derive_key(cipher, password, salt, key_buffer)) {
        memset(key_buffer, 0, sizeof(key_buffer));
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        printf("Error: Could not allocate cipher context\n");
        memset(key_buffer, 0, sizeof(key_buffer));
        return -1;
    }

    int status = -1;
    int current_length = 0;
    int total_length = 0;

    if (EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL) != 1) {
        printf("Error: Could not initialise decryption operation\n");
        goto cleanup;
    }

    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key_buffer, iv_length > 0 ? iv : NULL) != 1) {
        printf("Error: Could not set key and IV for decryption\n");
        goto cleanup;
    }

    if (EVP_DecryptUpdate(ctx, plain_text, &current_length, ciphertext, cipher_text_length) != 1) {
        printf("Error: Could not decrypt data\n");
        goto cleanup;
    }
    total_length = current_length;

    if (EVP_DecryptFinal_ex(ctx, plain_text + total_length, &current_length) != 1) {
        printf("Error: Could not finalise decryption (padding mismatch?)\n");
        goto cleanup;
    }
    total_length += current_length;

    status = total_length;

cleanup:
    EVP_CIPHER_CTX_free(ctx);
    memset(key_buffer, 0, sizeof(key_buffer));
    return status;
}
