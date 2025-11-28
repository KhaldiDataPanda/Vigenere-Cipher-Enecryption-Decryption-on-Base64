#ifndef CIPHER_LIB_H
#define CIPHER_LIB_H

#include <stddef.h>

// Get the index of a character in base64 alphabet
// Returns -1 for padding character '=' or invalid characters
int get_base64_index(char c);

// Apply Vigenère cipher to base64-encoded content
// Returns encrypted content (still Base64 string), or NULL on error
// output_len will contain the length of the output
char* vigenere_cipher(const char* input, size_t input_len, const char* key, size_t key_len, size_t* output_len);

// Reverse Vigenère cipher on base64-encoded content
// Returns decrypted content (Base64 string), or NULL on error
// output_len will contain the length of the output
char* vigenere_decipher(const char* input, size_t input_len, const char* key, size_t key_len, size_t* output_len);

// Find the key used for encryption by comparing plaintext and ciphertext
// Returns the key, or NULL on error
// key_len will contain the length of the key
char* find_key(const char* plaintext, size_t plain_len, const char* ciphertext, size_t cipher_len, size_t* key_len);

// Base64 Encode: Converts raw binary bytes into a Base64 string
// Returns the Base64 string (null-terminated), or NULL on error
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length);

// Base64 Decode: Converts a Base64 string into raw binary bytes
// Returns the raw bytes, or NULL on error
unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length);

#endif // CIPHER_LIB_H
