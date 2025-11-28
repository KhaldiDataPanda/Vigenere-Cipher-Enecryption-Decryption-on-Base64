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

## Testing Workflow

You can verify the correctness of the C programs using the following workflow:

```bash
# 1. Create a test file
echo "Hello World" > test.txt

# 2. Encode it to Base64
base64 test.txt > test.b64

# 3. Encrypt it (this modifies test.b64)
./cipher MYKEY test.b64

# 4. Try to find the key (Known Plaintext Attack)
# Note: You need the original Base64 for this. 
# In a real scenario, you'd have a backup or a known header.
# For this test, let's assume we have the original:
echo "Hello World" | base64 > original.b64
./findkey original.b64 test.b64
# Output should be: MYKEY

# 5. Decrypt it (this modifies test.b64 back)
./decipher MYKEY test.b64

# 6. Verify the result
# Note: diff -w ignores whitespace differences (like trailing newlines)
diff -w original.b64 test.b64
# If no output, the files match!

# 7. Decode back to text
base64 -d test.b64
# Output should be: Hello World
```

## Bash Toolbox Usage

The project includes a set of Bash scripts for forensic analysis.

### 1. Initialize the Toolbox
Sets up the `.sh-toolbox` directory to track archives.
```bash
./init-toolbox.sh
```

### 2. Import Archives
Imports `.gz` archives into the toolbox.
```bash
./import-archive.sh /path/to/archive.tar.gz
```

### 3. List Archives
Lists all tracked archives and their status.
```bash
./ls-toolbox.sh
```

### 4. Analyze an Archive
Analyzes a specific archive to find the attack timestamp and modified files.
```bash
./check-archive.sh
```
