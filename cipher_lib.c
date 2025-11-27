#include "cipher_lib.h"
#include <stdlib.h>
#include <string.h>



// Base64 alphabet
const char BASE64_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Get the index of a character in base64 alphabet
int get_base64_index(char c) {
    if (c == '=') return -1; // padding character
    
    for (int i = 0; i < 64; i++) {
        if (BASE64_ALPHABET[i] == c) {
            return i;
        }
    }
    return -1; // invalid character
}




// Apply Vigenère cipher to base64-encoded content
char* vigenere_cipher(const char* input, size_t input_len, const char* key, size_t key_len, size_t* output_len) {
    char* output = malloc(input_len + 1);
    if (!output) return NULL;
    
    size_t key_pos = 0;
    size_t out_pos = 0;
    
    for (size_t i = 0; i < input_len; i++) {
        int char_index = get_base64_index(input[i]);
        
        // Skip padding characters
        if (char_index == -1) {
            output[out_pos++] = input[i];
            continue;
        }
        
        // Find next valid key character (skip padding), cycling through key
        int key_index = -1;
        size_t attempts = 0;
        while (key_index == -1 && attempts < key_len) {
            key_index = get_base64_index(key[key_pos % key_len]);
            if (key_index == -1) {
                key_pos++;
                attempts++;
            }
        }
        
        if (key_index == -1) {
            free(output);
            return NULL; // no valid key characters
        }
        
        // Apply Vigenère cipher
        int encrypted_index = (char_index + key_index) % 64;
        output[out_pos++] = BASE64_ALPHABET[encrypted_index];
        
        key_pos++;
    }
    
    output[out_pos] = '\0';
    *output_len = out_pos;
    return output;
}




// Reverse Vigenère cipher on base64-encoded content
char* vigenere_decipher(const char* input, size_t input_len, const char* key, size_t key_len, size_t* output_len) {
    char* output = malloc(input_len + 1);
    if (!output) return NULL;
    
    size_t key_pos = 0;
    size_t out_pos = 0;
    
    for (size_t i = 0; i < input_len; i++) {
        int char_index = get_base64_index(input[i]);
        
        // Skip padding characters
        if (char_index == -1) {
            output[out_pos++] = input[i];
            continue;
        }
        
        // Find next valid key character (skip padding), cycling through key
        int key_index = -1;
        size_t attempts = 0;
        while (key_index == -1 && attempts < key_len) {
            key_index = get_base64_index(key[key_pos % key_len]);
            if (key_index == -1) {
                key_pos++;
                attempts++;
            }
        }
        
        if (key_index == -1) {
            free(output);
            return NULL; // no valid key characters
        }
        
        // Apply reverse Vigenère cipher
        int decrypted_index = (char_index - key_index + 64) % 64;
        output[out_pos++] = BASE64_ALPHABET[decrypted_index];
        
        key_pos++;
    }
    
    output[out_pos] = '\0';
    *output_len = out_pos;
    return output;
}



// Find the minimal repeating pattern in a string
size_t find_minimal_period(const char* str, size_t len) {
    for (size_t period = 1; period <= len / 2; period++) {
        int is_period = 1;
        for (size_t i = period; i < len; i++) {
            if (str[i] != str[i % period]) {
                is_period = 0;
                break;
            }
        }
        if (is_period) {
            return period;
        }
    }
    return len;
}




// Find the key used for encryption
char* find_key(const char* plaintext, size_t plain_len, const char* ciphertext, size_t cipher_len, size_t* key_len) {
    // Key length is determined by the number of non-padding characters
    size_t max_len = (plain_len < cipher_len) ? plain_len : cipher_len;
    char* full_key = malloc(max_len + 1);
    if (!full_key) return NULL;
    
    size_t key_pos = 0;
    size_t plain_pos = 0;
    size_t cipher_pos = 0;
    
    // Extract key by comparing plaintext and ciphertext
    while (plain_pos < plain_len && cipher_pos < cipher_len) {
        int plain_index = get_base64_index(plaintext[plain_pos]);
        int cipher_index = get_base64_index(ciphertext[cipher_pos]);
        
        // Skip padding characters
        if (plain_index == -1) {
            plain_pos++;
            continue;
        }
        if (cipher_index == -1) {
            cipher_pos++;
            continue;
        }
        
        // Calculate key character: K = (C - P + 64) mod 64
        int key_index = (cipher_index - plain_index + 64) % 64;
        full_key[key_pos++] = BASE64_ALPHABET[key_index];
        
        plain_pos++;
        cipher_pos++;
    }
    
    full_key[key_pos] = '\0';
    
    // Find minimal repeating pattern
    size_t minimal_period = find_minimal_period(full_key, key_pos);
    
    // Create minimal key
    char* key = malloc(minimal_period + 1);
    if (!key) {
        free(full_key);
        return NULL;
    }
    
    strncpy(key, full_key, minimal_period);
    key[minimal_period] = '\0';
    free(full_key);
    
    *key_len = minimal_period;
    return key;


    /* IMPORTANT NOTE : The key length is a must, without it we cannot be sure of the sequnce of the key
    */
}
