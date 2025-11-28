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
    
    // Read file content (Binary)
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
    
    unsigned char* content = malloc(file_size);
    if (!content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    fclose(file);
    
    // 1. Convert binary bytes back into a Base64 string
    size_t base64_len;
    char* base64_output = base64_encode(content, read_size, &base64_len);
    free(content);
    
    if (!base64_output) {
        fprintf(stderr, "Error: Base64 encoding failed\n");
        return 1;
    }
    
    // 2. Perform Vigenère subtraction (Base64 -> Base64)
    size_t decrypted_len;
    char* decrypted_output = vigenere_decipher(base64_output, base64_len, key, key_len, &decrypted_len);
    free(base64_output);
    
    if (!decrypted_output) {
        fprintf(stderr, "Error: Decryption failed\n");
        return 1;
    }
    
    // 3. Overwrite the input file with the resulting Base64 string
    file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Cannot write to file %s\n", filename);
        free(decrypted_output);
        return 1;
    }
    
    fwrite(decrypted_output, 1, decrypted_len, file);
    fclose(file);
    free(decrypted_output);
    
    return 0;
}
