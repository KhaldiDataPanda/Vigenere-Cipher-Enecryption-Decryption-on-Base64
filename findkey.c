#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cipher_lib.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <plaintext_file> <encrypted_file>\n", argv[0]);
        return 1;
    }
    
    const char* plaintext_file = argv[1];
    const char* encrypted_file = argv[2];
    
    // Read plaintext file
    FILE* file = fopen(plaintext_file, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", plaintext_file);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long plain_size = ftell(file);
    if (plain_size < 0) {
        fprintf(stderr, "Error: Cannot determine file size\n");
        fclose(file);
        return 1;
    }
    fseek(file, 0, SEEK_SET);
    
    char* plaintext = malloc(plain_size + 1);
    if (!plaintext) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    size_t plain_len = fread(plaintext, 1, plain_size, file);
    plaintext[plain_len] = '\0';
    fclose(file);
    
    // Read encrypted file
    file = fopen(encrypted_file, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", encrypted_file);
        free(plaintext);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long cipher_size = ftell(file);
    if (cipher_size < 0) {
        fprintf(stderr, "Error: Cannot determine file size\n");
        fclose(file);
        free(plaintext);
        return 1;
    }
    fseek(file, 0, SEEK_SET);
    
    char* ciphertext = malloc(cipher_size + 1);
    if (!ciphertext) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        free(plaintext);
        return 1;
    }
    
    size_t cipher_len = fread(ciphertext, 1, cipher_size, file);
    ciphertext[cipher_len] = '\0';
    fclose(file);
    
    // Find the key
    size_t key_len;
    char* key = find_key(plaintext, plain_len, ciphertext, cipher_len, &key_len);
    free(plaintext);
    free(ciphertext);
    
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
