# Project Explanation: Vigenère Cipher on Base64

This document provides a comprehensive explanation of the Vigenère Cipher project. It details the theoretical background of the algorithms used and directly correlates them with the C code implementation found in `cipher_lib.c`.

## 1. Project Overview

The goal of this project is to implement a **Vigenère Cipher** that operates on **Base64-encoded** files.
The project includes:
-   **Encryption**: Transforming plaintext into ciphertext using a key.
-   **Decryption**: Reversing the process to recover the original text.
-   **Cryptanalysis**: Recovering the key when both plaintext and ciphertext are known (Known-Plaintext Attack).

---

## 2. The Base64 Alphabet

### Theory
Unlike the classical Vigenère cipher which uses the 26 letters of the alphabet, this implementation uses the **Base64** alphabet. This alphabet consists of 64 printable characters:
-   `A`–`Z` (Indices 0–25)
-   `a`–`z` (Indices 26–51)
-   `0`–`9` (Indices 52–61)
-   `+` (Index 62)
-   `/` (Index 63)

The character `=` is used for padding in Base64 and is **not** part of the encryption alphabet. It must be preserved or skipped but not encrypted.

### Code Implementation
The alphabet is defined as a constant string. A helper function `get_base64_index` is used to map a character to its integer value (0-63).

```c
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
```

---

## 3. Encryption

### Theory
The Vigenère cipher is a polyalphabetic substitution cipher.
Let:
-   $P$ be the index of the plaintext character.
-   $K$ be the index of the key character.
-   $C$ be the index of the resulting ciphertext character.
-   $N = 64$ is the size of the alphabet.

The encryption formula for a single character is:
$$ C = (P + K) \pmod{64} $$

The key is repeated cyclically to match the length of the plaintext. If the key is "KEY" and the text is "HELLO", the key stream becomes "KEYKE".

### Code Implementation
The `vigenere_cipher` function implements this logic.
**Key features:**
1.  It skips padding (`=`) and invalid characters in the input.
2.  It cycles through the key using modulo arithmetic (`key_pos % key_len`).
3.  It handles the case where the key itself might contain padding characters (though a valid key shouldn't).

```c
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
        
        // Apply Vigenère cipher: C = (P + K) % 64
        int encrypted_index = (char_index + key_index) % 64;
        output[out_pos++] = BASE64_ALPHABET[encrypted_index];
        
        key_pos++;
    }
    
    output[out_pos] = '\0';
    *output_len = out_pos;
    return output;
}
```

---

## 4. Decryption

### Theory
Decryption is the inverse of encryption. We need to subtract the key value from the ciphertext value.
$$ P = (C - K) \pmod{64} $$

In C, the modulo operator `%` can return negative values for negative operands. To ensure a positive result (a valid index between 0 and 63), we add the modulus $N=64$ before applying the operator:
$$ P = (C - K + 64) \pmod{64} $$

### Code Implementation
The `vigenere_decipher` function mirrors the encryption function but uses the subtraction formula.

```c
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
        
        // Apply reverse Vigenère cipher: P = (C - K + 64) % 64
        int decrypted_index = (char_index - key_index + 64) % 64;
        output[out_pos++] = BASE64_ALPHABET[decrypted_index];
        
        key_pos++;
    }
    
    output[out_pos] = '\0';
    *output_len = out_pos;
    return output;
}
```

---

## 5. Key Recovery (Cryptanalysis)

### Theory
If we possess both the **Plaintext ($P$)** and the **Ciphertext ($C$)**, we can recover the key. This is known as a Known-Plaintext Attack.

From the encryption formula $C = (P + K) \pmod{64}$, we can isolate $K$:
$$ K = (C - P + 64) \pmod{64} $$

By applying this to the entire file, we recover the **Key Stream** (e.g., `SECRETKEYSECRETKEY...`).
The actual **Key** is the shortest repeating pattern (period) within this stream.

### Code Implementation

#### Step 1: Recover the Key Stream
The `find_key` function first generates the full key stream by comparing $P$ and $C$ character by character.

```c
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
```

#### Step 2: Find the Minimal Period
Once we have the long string `full_key`, we need to find the shortest repeating unit. The function `find_minimal_period` checks every possible period length starting from 1.

```c
// Find the minimal repeating pattern in a string
size_t find_minimal_period(const char* str, size_t len) {
    for (size_t period = 1; period <= len / 2; period++) {
        int is_period = 1;
        // Check if the string repeats every 'period' characters
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
    return len; // No smaller period found
}
```

#### Step 3: Final Key Extraction
Finally, `find_key` uses the discovered period to truncate the full key stream to just the actual key.

```c
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
```

---

## 6. Program Usage Summary

### Compilation
```bash
make all
```

### Execution
1.  **Encrypt**: `./cipher <key> <file>`
2.  **Decrypt**: `./decipher <key> <file>`
3.  **Find Key**: `./findkey <plaintext_file> <encrypted_file>`
