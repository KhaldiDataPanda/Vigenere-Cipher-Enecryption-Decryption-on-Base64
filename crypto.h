#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>
#include <stdint.h>

// Base64 characters
extern const char base64_chars[];

// Function prototypes
int base64_char_index(char c);
char base64_index_char(int index);
void vigenere_encrypt(char *data, size_t data_len, const char *key, size_t key_len);
void vigenere_decrypt(char *data, size_t data_len, const char *key, size_t key_len);
unsigned char *base64_decode(const char *data, size_t input_len, size_t *output_len);
char *base64_encode(const unsigned char *data, size_t input_len, size_t *output_len);

#endif // CRYPTO_H
