#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cipher_lib.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <key> <filename>\n", argv[0]);
        return 1;
    }
    
    const char* key = argv[1];
    const char* filename = argv[2];
    size_t key_len = strlen(key);
    
    if (key_len == 0) {
        fprintf(stderr, "Error: Empty key\n");
        return 1;
    }
    
    // Read file content (Base64 text)
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        fprintf(stderr, "Error: Cannot determine file size\n");
        fclose(file);
        return 1;
    }
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(file_size + 1);
    if (!content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0';
    fclose(file);
    
    // 1. Apply Vigenère cipher (Base64 -> Base64)
    size_t vigenere_len;
    char* vigenere_output = vigenere_cipher(content, read_size, key, key_len, &vigenere_len);
    free(content);
    
    if (!vigenere_output) {
        fprintf(stderr, "Error: Encryption failed\n");
        return 1;
    }
    
    // 2. Decode from Base64 to raw binary bytes
    size_t binary_len;
    unsigned char* binary_output = base64_decode(vigenere_output, vigenere_len, &binary_len);
    free(vigenere_output);
    
    if (!binary_output) {
        fprintf(stderr, "Error: Base64 decoding failed\n");
        return 1;
    }
    
    // 3. Overwrite the original file with raw bytes
    file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Cannot write to file %s\n", filename);
        free(binary_output);
        return 1;
    }
    
    fwrite(binary_output, 1, binary_len, file);
    fclose(file);
    free(binary_output);
    
    return 0;
}
