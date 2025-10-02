#ifndef STEGOBMP_CRYPTO_H
#define STEGOBMP_CRYPTO_H

#define CRYPTO_SALT_SIZE 8
#define CRYPTO_AES_IV_SIZE 16
#define CRYPTO_3DES_IV_SIZE 32
#define CRYPTO_MAX_IV_SIZE 16

int crypto_encrypt(
    const unsigned char *plain_text,
    int plain_tex_lenght,
    const char *method,
    const char *mode,
    const char *password,
    const unsigned char *salt,
    const unsigned char *iv,
    unsigned char *ciphertext
    );

int crypto_decrypt(
    const unsigned char *ciphertext,
    int cipher_text_length,
    const char *method,
    const char *mode,
    const char *password,
    const unsigned char *salt,
    const unsigned char *iv,
    unsigned char *plain_text
    );

#endif //STEGOBMP_CRYPTO_H