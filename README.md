# Vigenère Cipher Programs for Base64-Encoded Files

This directory contains three programs that implement Vigenère cipher encryption/decryption for base64-encoded files.

## Programs

### cipher
Encrypts a base64-encoded file using Vigenère cipher with a base64 key.

**Usage:**
```bash
./cipher <key> <filename>
```

**Example:**
```bash
# First encode your file to base64
base64 myfile.txt > myfile_b64.txt

# Then encrypt it
./cipher MySecretKey myfile_b64.txt
```

The encrypted content replaces the original file content.

### decipher
Decrypts a base64-encoded file that was encrypted with the Vigenère cipher.

**Usage:**
```bash
./decipher <key> <filename>
```

**Example:**
```bash
./decipher MySecretKey myfile_b64.txt

# Then decode from base64 to get the original content
base64 -d myfile_b64.txt > myfile.txt
```

The decrypted content replaces the original file content.

### findkey
Determines the encryption key by comparing a plaintext file with its encrypted version.

**Usage:**
```bash
./findkey <plaintext_file> <encrypted_file>
```

**Example:**
```bash
./findkey plaintext_b64.txt encrypted_b64.txt
```

The key is printed to stdout, and the key length is printed to stderr.

## Building

### Build all programs
```bash
make all
```

### Build individual programs
```bash
make cipher
make decipher
make findkey
```

### Build static library (bonus)
```bash
make library
```

This creates `libcipher.a` which contains the core encryption/decryption functions that can be linked into other programs.

### Clean build artifacts
```bash
make clean
```

## Library Usage

The static library `libcipher.a` provides the following functions (see `cipher_lib.h`):

- `vigenere_cipher()` - Encrypt base64-encoded content
- `vigenere_decipher()` - Decrypt base64-encoded content
- `find_key()` - Determine encryption key from plaintext and ciphertext
- `find_minimal_period()` - Find minimal repeating pattern in a key
- `get_base64_index()` - Get index of a character in base64 alphabet

To use the library in your own program:
```c
#include "cipher_lib.h"

// Link with: gcc -o myprogram myprogram.c -L. -lcipher
```

## Implementation Details

The encryption process follows these steps:
1. Input file is already encoded in base64
2. Apply Vigenère cipher with a base64 key (padding characters '=' are skipped)
3. Output is written back to the file

The Vigenère cipher uses the base64 alphabet (A-Z, a-z, 0-9, +, /) as a 64-character alphabet:
- Encryption: `C[i] = (P[i] + K[i mod keylen]) mod 64`
- Decryption: `P[i] = (C[i] - K[i mod keylen] + 64) mod 64`

The `findkey` program extracts the minimal repeating key pattern from the full key sequence.
