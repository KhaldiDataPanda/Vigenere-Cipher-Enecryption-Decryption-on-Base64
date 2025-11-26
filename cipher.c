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
    
    // Read file content
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return 1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        fprintf(stderr, "Error: Cannot determine file size\n");
        fclose(file);
        return 1;
    }
    fseek(file, 0, SEEK_SET);
    
    // Read content
    char* content = malloc(file_size + 1);
    if (!content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0';
    fclose(file);
    
    // Apply cipher
    size_t output_len;
    char* encrypted = vigenere_cipher(content, read_size, key, key_len, &output_len);
    free(content);
    
    if (!encrypted) {
        fprintf(stderr, "Error: Encryption failed\n");
        return 1;
    }
    
    // Write encrypted content back to file
    file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Cannot write to file %s\n", filename);
        free(encrypted);
        return 1;
    }
    
    fwrite(encrypted, 1, output_len, file);
    fclose(file);
    free(encrypted);
    
    return 0;
}
