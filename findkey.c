#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cipher_lib.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <clear_file> <encrypted_file>\n", argv[0]);
        return 1;
    }
    
    const char* clear_file = argv[1];
    const char* encrypted_file = argv[2];
    
    // Read Clear file (Base64)
    FILE* file = fopen(clear_file, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", clear_file);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long clear_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* clear_text = malloc(clear_size + 1);
    if (!clear_text) {
        fclose(file);
        return 1;
    }
    size_t clear_len = fread(clear_text, 1, clear_size, file);
    clear_text[clear_len] = '\0';
    fclose(file);
    
    // Read Encrypted file (Binary)
    file = fopen(encrypted_file, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", encrypted_file);
        free(clear_text);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long enc_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    unsigned char* enc_bytes = malloc(enc_size);
    if (!enc_bytes) {
        fclose(file);
        free(clear_text);
        return 1;
    }
    size_t enc_len = fread(enc_bytes, 1, enc_size, file);
    fclose(file);
    
    // Convert Encrypted file (Binary) back to Base64
    size_t enc_base64_len;
    char* enc_base64 = base64_encode(enc_bytes, enc_len, &enc_base64_len);
    free(enc_bytes);
    
    if (!enc_base64) {
        fprintf(stderr, "Error: Base64 encoding failed\n");
        free(clear_text);
        return 1;
    }
    
    // Find the key
    size_t key_len;
    char* key = find_key(clear_text, clear_len, enc_base64, enc_base64_len, &key_len);
    
    free(clear_text);
    free(enc_base64);
    
    if (!key) {
        fprintf(stderr, "Error: Key finding failed\n");
        return 1;
    }
    
    // Output key to stdout
    printf("%s", key);
    
    // Output key length to stderr
    fprintf(stderr, "%zu\n", key_len);
    
    free(key);
    return 0;
}
