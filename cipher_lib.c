#include "cipher_lib.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
        
        // If it's padding or invalid (like newline), just copy it
        if (char_index == -1) {
            output[out_pos++] = input[i];
            continue;
        }
        
        // Find next valid key character (skip padding), cycling through key
        int key_index = -1;
        size_t attempts = 0;
        while (key_index == -1 && attempts < key_len * 2) { 
            char key_char = key[key_pos % key_len];
            key_index = get_base64_index(key_char);
            if (key_index == -1) {
                key_pos++;
                attempts++;
            }
        }
        
        if (key_index == -1) {
            key_index = 0; 
        }

        // Apply Vigenère cipher
        int encrypted_index = (char_index + key_index) % 64;
        output[out_pos++] = BASE64_ALPHABET[encrypted_index];
        
        // Advance key only if we used it
        key_pos++;
    }
    
    output[out_pos] = '\0';
    if (output_len) *output_len = out_pos;
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
        
        // Skip padding characters or invalid chars
        if (char_index == -1) {
            output[out_pos++] = input[i];
            continue;
        }
        
        // Find next valid key character
        int key_index = -1;
        size_t attempts = 0;
        while (key_index == -1 && attempts < key_len * 2) {
            char key_char = key[key_pos % key_len];
            key_index = get_base64_index(key_char);
            if (key_index == -1) {
                key_pos++;
                attempts++;
            }
        }
        
        if (key_index == -1) key_index = 0;

        // Apply Reverse Vigenère cipher
        int decrypted_index = (char_index - key_index) % 64;
        if (decrypted_index < 0) decrypted_index += 64;
        
        output[out_pos++] = BASE64_ALPHABET[decrypted_index];
        
        key_pos++;
    }
    
    output[out_pos] = '\0';
    if (output_len) *output_len = out_pos;
    return output;
}

// Find the key used for encryption
char* find_key(const char* plaintext, size_t plain_len, const char* ciphertext, size_t cipher_len, size_t* key_len) {
    size_t min_len = (plain_len < cipher_len) ? plain_len : cipher_len;
    
    char* key_stream = malloc(min_len + 1);
    if (!key_stream) return NULL;
    
    size_t k_idx = 0;
    
    for (size_t i = 0; i < min_len; i++) {
        int p_idx = get_base64_index(plaintext[i]);
        int c_idx = get_base64_index(ciphertext[i]);
        
        if (p_idx != -1 && c_idx != -1) {
            int k_val = (c_idx - p_idx) % 64;
            if (k_val < 0) k_val += 64;
            key_stream[k_idx++] = BASE64_ALPHABET[k_val];
        }
    }
    key_stream[k_idx] = '\0';
    
    size_t best_period = k_idx; 
    
    for (size_t p = 1; p <= k_idx / 2; p++) {
        int match = 1;
        for (size_t i = 0; i < k_idx - p; i++) {
            if (key_stream[i] != key_stream[i+p]) {
                match = 0;
                break;
            }
        }
        if (match) {
            best_period = p;
            break;
        }
    }
    
    char* final_key = malloc(best_period + 1);
    if (final_key) {
        strncpy(final_key, key_stream, best_period);
        final_key[best_period] = '\0';
        *key_len = best_period;
    }
    
    free(key_stream);
    return final_key;
}

// Base64 Encode
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    size_t encoded_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = malloc(encoded_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = BASE64_ALPHABET[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = BASE64_ALPHABET[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = BASE64_ALPHABET[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = BASE64_ALPHABET[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < (3 - input_length % 3) % 3; i++)
        encoded_data[encoded_length - 1 - i] = '=';

    encoded_data[encoded_length] = '\0';
    if (output_length) *output_length = encoded_length;
    return encoded_data;
}

// Base64 Decode
unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length) {
    size_t valid_len = 0;
    for(size_t i=0; i<input_length; i++) {
        if(get_base64_index(data[i]) != -1 || data[i] == '=') {
            valid_len++;
        }
    }
    
    if (valid_len % 4 != 0) {
        // Handle error or just proceed?
    }

    size_t decoded_length = valid_len / 4 * 3;
    // Check padding at the end of the valid string
    // We need to find the last valid chars to check for padding
    int padding = 0;
    if (valid_len > 0) {
        // Scan backwards for padding
        // This is a bit tricky if there are newlines at the end.
        // Let's just assume standard padding handling during the loop.
    }

    unsigned char* decoded_data = malloc(decoded_length + 3); // +3 for safety
    if (decoded_data == NULL) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < input_length;) {
        int sextet_a = -1, sextet_b = -1, sextet_c = -1, sextet_d = -1;
        
        while (i < input_length && sextet_a == -1) {
             char c = data[i++];
             if (c == '=') sextet_a = 0; 
             else sextet_a = get_base64_index(c);
        }
        while (i < input_length && sextet_b == -1) {
             char c = data[i++];
             if (c == '=') sextet_b = 0; 
             else sextet_b = get_base64_index(c);
        }
        while (i < input_length && sextet_c == -1) {
             char c = data[i++];
             if (c == '=') { sextet_c = 0; padding++; }
             else sextet_c = get_base64_index(c);
        }
        while (i < input_length && sextet_d == -1) {
             char c = data[i++];
             if (c == '=') { sextet_d = 0; padding++; }
             else sextet_d = get_base64_index(c);
        }

        if (sextet_a == -1 || sextet_b == -1 || sextet_c == -1 || sextet_d == -1) break;

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
    
    // Adjust length based on padding found
    if (output_length) *output_length = j - padding;
    return decoded_data;
}
