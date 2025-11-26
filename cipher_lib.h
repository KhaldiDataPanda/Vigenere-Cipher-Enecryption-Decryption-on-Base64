#ifndef CIPHER_LIB_H
#define CIPHER_LIB_H

#include <stddef.h>

// Get the index of a character in base64 alphabet
// Returns -1 for padding character '=' or invalid characters
int get_base64_index(char c);

// Apply Vigenère cipher to base64-encoded content
// Returns encrypted content, or NULL on error
// output_len will contain the length of the output
char* vigenere_cipher(const char* input, size_t input_len, const char* key, size_t key_len, size_t* output_len);

// Reverse Vigenère cipher on base64-encoded content
// Returns decrypted content, or NULL on error
// output_len will contain the length of the output
char* vigenere_decipher(const char* input, size_t input_len, const char* key, size_t key_len, size_t* output_len);

// Find the minimal repeating pattern in a string
// Returns the length of the minimal period
size_t find_minimal_period(const char* str, size_t len);

// Find the key used for encryption by comparing plaintext and ciphertext
// Returns the key, or NULL on error
// key_len will contain the length of the key
char* find_key(const char* plaintext, size_t plain_len, const char* ciphertext, size_t cipher_len, size_t* key_len);

#endif // CIPHER_LIB_H
