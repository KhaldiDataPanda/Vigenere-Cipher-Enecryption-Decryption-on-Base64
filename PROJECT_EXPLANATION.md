# Project Explanation: Vigenère Cipher on Base64

This document provides a comprehensive explanation of the Vigenère Cipher project. It details the theoretical background of the algorithms used and directly correlates them with the C code implementation.

## 1. Project Overview

This project simulates a cybersecurity scenario where we must develop tools to handle a specific encryption method and analyze compromised system files.

### 1.1 The Scenario

**Part 1: Cryptographic Analysis (C Language)**
Our team has identified a custom encryption scheme used in a recent attack. To recover the files, we need to reverse the process. The encryption follows this specific sequence:
1.  **Base64 Encoding**: The original file is encoded into Base64.
2.  **Vigenère Cipher**: The Base64 content is encrypted using a Vigenère cipher. Crucially, padding characters (`=`) are skipped during this process.
3.  **Base64 Decoding**: The encrypted Base64 string is decoded back to binary (conceptually, though our tools mostly work on the Base64 layer).

We need to build a suite of C programs to replicate this encryption, perform decryption, and recover keys from known plaintext/ciphertext pairs.

**Part 2: Forensic Investigation (Bash Scripting)**
Following the attack, we receive compressed archives (`.gz`) containing system files from compromised machines. These archives contain:
-   Authentication logs (`auth.log`).
-   User data files.

We need to build a **Bash Toolbox** to:
1.  **Manage Evidence**: Organize and track these archives in a structured workspace (`.sh-toolbox`).
2.  **Analyze Traces**: Parse logs to find when the attacker (`admin` user) logged in, and identify which files were modified after that timestamp.

### 1.2 Project Components

The project is divided into two main technical areas:

-   **C Part (Cryptography)**:
    -   **Encryption (`cipher`)**: Transforming plaintext into ciphertext using a key.
    -   **Decryption (`decipher`)**: Reversing the process to recover the original text.
    -   **Cryptanalysis (`findkey`)**: Recovering the key when both plaintext and ciphertext are known (Known-Plaintext Attack).

-   **Bash Part (Forensics)**:
    -   **Toolbox Management**: Initializing (`init-toolbox.sh`) and maintaining a workspace.
    -   **Archive Handling**: Importing (`import-archive.sh`) and listing (`ls-toolbox.sh`) compressed archives.
    -   **Forensics**: Analyzing archives (`check-archive.sh`) to identify attack timestamps and impacted files.

### Project Files
| File | Description |
|------|-------------|
| `cipher_lib.c` | Core library containing all cipher functions |
| `cipher.c` | Main program for encryption |
| `decipher.c` | Main program for decryption |
| `findkey.c` | Main program for key recovery |
| `Makefile` | Build automation script |
| `init-toolbox.sh` | Initializes the workspace environment |
| `import-archive.sh` | Imports archives into the toolbox |
| `ls-toolbox.sh` | Lists available archives |
| `check-archive.sh` | Analyzes an archive for attack details |

---

## 2. The Library File: `cipher_lib.c`

This file contains all the core cryptographic functions used by the main programs. It implements the Vigenère cipher operations on the Base64 alphabet.

### 2.1 The Base64 Alphabet

#### Theory
Unlike the classical Vigenère cipher which uses the 26 letters of the alphabet, this implementation uses the **Base64** alphabet. This alphabet consists of 64 printable characters:
-   `A`–`Z` (Indices 0–25)
-   `a`–`z` (Indices 26–51)
-   `0`–`9` (Indices 52–61)
-   `+` (Index 62)
-   `/` (Index 63)

The character `=` is used for padding in Base64 and is **not** part of the encryption alphabet. It must be preserved or skipped but not encrypted.

#### Code Implementation
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

### 2.2 The Encryption Function: `vigenere_cipher`

#### Theory
The Vigenère cipher is a polyalphabetic substitution cipher.
Let:
-   $P$ be the index of the plaintext character.
-   $K$ be the index of the key character.
-   $C$ be the index of the resulting ciphertext character.
-   $N = 64$ is the size of the alphabet.

The encryption formula for a single character is:
$$ C = (P + K) \pmod{64} $$

The key is repeated cyclically to match the length of the plaintext. If the key is "KEY" and the text is "HELLO", the key stream becomes "KEYKE".

#### Code Implementation
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
        while (key_index == -1 && attempts < key_len * 2) {
            char key_char = key[key_pos % key_len];
            key_index = get_base64_index(key_char);
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

### 2.3 The Decryption Function: `vigenere_decipher`

#### Theory
Decryption is the inverse of encryption. We need to subtract the key value from the ciphertext value.
$$ P = (C - K) \pmod{64} $$

In C, the modulo operator `%` can return negative values for negative operands. To ensure a positive result (a valid index between 0 and 63), we add the modulus $N=64$ before applying the operator:
$$ P = (C - K + 64) \pmod{64} $$

#### Code Implementation
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
        while (key_index == -1 && attempts < key_len * 2) {
            char key_char = key[key_pos % key_len];
            key_index = get_base64_index(key_char);
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

### 2.4 Key Recovery Functions (Cryptanalysis)

#### Theory
If we possess both the **Plaintext ($P$)** and the **Ciphertext ($C$)**, we can recover the key. This is known as a Known-Plaintext Attack.

From the encryption formula $C = (P + K) \pmod{64}$, we can isolate $K$:
$$ K = (C - P + 64) \pmod{64} $$

By applying this to the entire file, we recover the **Key Stream** (e.g., `SECRETKEYSECRETKEY...`).
The actual **Key** is the shortest repeating pattern (period) within this stream.

#### Code Implementation

##### The `find_minimal_period` Function
This helper function finds the shortest repeating pattern in a string. It checks every possible period length starting from 1.

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

##### The `find_key` Function
This function recovers the encryption key by:
1. Comparing plaintext and ciphertext character by character
2. Computing each key character using the formula $K = (C - P + 64) \pmod{64}$
3. Finding the minimal repeating period of the recovered key stream

```c
// Find the key used for encryption
char* find_key(const char* plaintext, size_t plain_len, const char* ciphertext, size_t cipher_len, size_t* key_len) {
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
}
```

---

## 3. The Encryption Program: `cipher.c`

This file contains the main program for encrypting files using the Vigenère cipher.

### Usage
```bash
./cipher <key> <filename>
```
-   `<key>`: The encryption key (a string of Base64 characters)
-   `<filename>`: The file to encrypt (will be modified in-place)

### Code Explanation

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cipher_lib.h"

int main(int argc, char* argv[]) {
```
-   **`#include` directives**: Include standard C libraries and the custom cipher library header.
-   **`int main(int argc, char* argv[])`**: The entry point of the program.
    -   `argc` (argument count): The number of command-line arguments passed to the program (including the program name itself).
    -   `argv` (argument vector): An array of strings containing the actual arguments.

```c
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <key> <filename>\n", argv[0]);
        return 1;
    }
```
-   **Argument validation**: The program expects exactly 3 arguments:
    -   `argv[0]`: The program name (e.g., `./cipher`)
    -   `argv[1]`: The encryption key
    -   `argv[2]`: The filename to encrypt
-   **`fprintf(stderr, ...)`**: Prints error messages to standard error stream.
-   **`return 1`**: Returns a non-zero exit code to indicate an error.

```c
    const char* key = argv[1];
    const char* filename = argv[2];
    size_t key_len = strlen(key);
    
    if (key_len == 0) {
        fprintf(stderr, "Error: Empty key\n");
        return 1;
    }
```
-   **Variable assignment**: Stores the key and filename from command-line arguments.
-   **`strlen(key)`**: Returns the length of the key string.
-   **Empty key check**: Ensures the key is not empty.

```c
    // Read file content
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return 1;
    }
```
-   **`fopen(filename, "rb")`**: Opens the file in binary read mode (`"rb"`).
    -   `"r"`: Read mode
    -   `"b"`: Binary mode (important for cross-platform compatibility)
-   **Error handling**: Checks if the file was opened successfully.

```c
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
```
-   **`fseek(file, 0, SEEK_END)`**: Moves the file pointer to the end of the file.
-   **`ftell(file)`**: Returns the current position (which is the file size in bytes).
-   **`fseek(file, 0, SEEK_SET)`**: Moves the file pointer back to the beginning.

```c
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
```
-   **`malloc(file_size + 1)`**: Allocates memory for the file content (+1 for null terminator).
-   **`fread(content, 1, file_size, file)`**: Reads `file_size` bytes from the file into `content`.
-   **`content[read_size] = '\0'`**: Null-terminates the string.
-   **`fclose(file)`**: Closes the file.

```c
    // Apply cipher
    size_t output_len;
    char* encrypted = vigenere_cipher(content, read_size, key, key_len, &output_len);
    free(content);
    
    if (!encrypted) {
        fprintf(stderr, "Error: Encryption failed\n");
        return 1;
    }
```
-   **`vigenere_cipher(...)`**: Calls the encryption function from the library.
-   **`free(content)`**: Frees the original content memory (no longer needed).
-   **Error handling**: Checks if encryption was successful.

```c
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
```
-   **`fopen(filename, "wb")`**: Opens the file in binary write mode (overwrites existing content).
-   **`fwrite(encrypted, 1, output_len, file)`**: Writes the encrypted content to the file.
-   **`free(encrypted)`**: Frees the encrypted content memory.
-   **`return 0`**: Returns success exit code.

---

## 4. The Decryption Program: `decipher.c`

This file contains the main program for decrypting files using the Vigenère cipher.

### Usage
```bash
./decipher <key> <filename>
```
-   `<key>`: The decryption key (must be the same key used for encryption)
-   `<filename>`: The encrypted file to decrypt (will be modified in-place)

### Code Explanation

The structure of `decipher.c` is nearly identical to `cipher.c`. The only significant difference is:

```c
    // Apply decipher
    size_t output_len;
    char* decrypted = vigenere_decipher(content, read_size, key, key_len, &output_len);
    free(content);
    
    if (!decrypted) {
        fprintf(stderr, "Error: Decryption failed\n");
        return 1;
    }
```
-   **`vigenere_decipher(...)`**: Calls the decryption function instead of the encryption function.

The rest of the file handling (reading, writing) is identical to `cipher.c`.

---

## 5. The Key Finder Program: `findkey.c`

This file contains the main program for recovering the encryption key using a known-plaintext attack.

### Usage
```bash
./findkey <plaintext_file> <encrypted_file>
```
-   `<plaintext_file>`: The original (unencrypted) Base64 file
-   `<encrypted_file>`: The encrypted version of the same file

### Output
-   **stdout**: The recovered key
-   **stderr**: The key length

### Code Explanation

```c
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <plaintext_file> <encrypted_file>\n", argv[0]);
        return 1;
    }
    
    const char* plaintext_file = argv[1];
    const char* encrypted_file = argv[2];
```
-   **Argument validation**: Expects the plaintext file and encrypted file as arguments.

```c
    // Read plaintext file
    FILE* file = fopen(plaintext_file, "rb");
    // ... (same file reading pattern as cipher.c)
    
    char* plaintext = malloc(plain_size + 1);
    size_t plain_len = fread(plaintext, 1, plain_size, file);
    plaintext[plain_len] = '\0';
    fclose(file);
```
-   **Read plaintext**: Opens and reads the original file into memory.

```c
    // Read encrypted file
    file = fopen(encrypted_file, "rb");
    // ... (same file reading pattern)
    
    char* ciphertext = malloc(cipher_size + 1);
    size_t cipher_len = fread(ciphertext, 1, cipher_size, file);
    ciphertext[cipher_len] = '\0';
    fclose(file);
```
-   **Read ciphertext**: Opens and reads the encrypted file into memory.

```c
    // Find the key
    size_t key_len;
    char* key = find_key(plaintext, plain_len, ciphertext, cipher_len, &key_len);
    free(plaintext);
    free(ciphertext);
    
    if (!key) {
        fprintf(stderr, "Error: Key finding failed\n");
        return 1;
    }
```
-   **`find_key(...)`**: Calls the key recovery function from the library.
-   **Memory cleanup**: Frees the plaintext and ciphertext buffers.

```c
    // Output key to stdout
    printf("%s", key);
    
    // Output key length to stderr
    fprintf(stderr, "%zu\n", key_len);
    
    free(key);
    return 0;
}
```
-   **`printf("%s", key)`**: Outputs the recovered key to stdout (for use in scripts).
-   **`fprintf(stderr, "%zu\n", key_len)`**: Outputs the key length to stderr (for debugging/information).
-   **`%zu`**: Format specifier for `size_t` type.

---

## 6. The Build System: `Makefile`

The Makefile automates the compilation process. It defines rules for building executables and managing dependencies.

### Makefile Syntax Overview

A Makefile consists of **rules** with this format:
```makefile
target: dependencies
	command
```
-   **target**: The file to create or action to perform
-   **dependencies**: Files that must exist (or be up-to-date) before building the target
-   **command**: Shell command to execute (MUST be indented with a TAB character)

### Variable Definitions

```makefile
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
AR = ar
ARFLAGS = rcs
```
-   **`CC = gcc`**: Sets the C compiler to GCC.
-   **`CFLAGS`**: Compiler flags:
    -   `-Wall`: Enable all common warnings
    -   `-Wextra`: Enable extra warnings
    -   `-std=c99`: Use the C99 standard
    -   `-O2`: Optimization level 2 (for faster code)
-   **`AR = ar`**: Archive utility for creating static libraries.
-   **`ARFLAGS = rcs`**: Archive flags:
    -   `r`: Replace/insert files
    -   `c`: Create archive if it doesn't exist
    -   `s`: Create an index (symbol table)

```makefile
# Targets
TARGETS = cipher decipher findkey
LIBRARY = libcipher.a
```
-   **`TARGETS`**: List of executable programs to build.
-   **`LIBRARY`**: Name of the static library file.

```makefile
# Source files
CIPHER_SRC = cipher.c
DECIPHER_SRC = decipher.c
FINDKEY_SRC = findkey.c

# Library source files 
LIB_SRC = cipher_lib.c
LIB_HDR = cipher_lib.h
```
-   **Source file variables**: Define which `.c` files correspond to each target.

```makefile
# Object files
CIPHER_OBJ = $(CIPHER_SRC:.c=.o)
DECIPHER_OBJ = $(DECIPHER_SRC:.c=.o)
FINDKEY_OBJ = $(FINDKEY_SRC:.c=.o)
LIB_OBJ = $(LIB_SRC:.c=.o)
```
-   **`$(VAR:.c=.o)`**: Pattern substitution - replaces `.c` extension with `.o`.
-   Example: `cipher.c` becomes `cipher.o`

### Build Rules

```makefile
# Default target: build all programs
all: $(TARGETS)
```
-   **`all`**: The default target (built when running `make` with no arguments).
-   Depends on all three executables.

```makefile
# Build cipher program (linked with library)
cipher: $(CIPHER_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(CIPHER_OBJ) -L. -lcipher
```
-   **Rule for `cipher`**: Depends on `cipher.o` and `libcipher.a`.
-   **Command breakdown**:
    -   `$(CC)`: Use the compiler (gcc)
    -   `$(CFLAGS)`: Apply compiler flags
    -   `-o $@`: Output file name (`$@` is the target name, i.e., `cipher`)
    -   `$(CIPHER_OBJ)`: Input object file
    -   `-L.`: Look for libraries in current directory
    -   `-lcipher`: Link with `libcipher.a` (the `lib` prefix and `.a` suffix are implicit)

```makefile
# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
```
-   **Pattern rule**: Defines how to build any `.o` file from a `.c` file.
-   **`%`**: Wildcard that matches any string.
-   **`$<`**: The first dependency (the `.c` file).
-   **`-c`**: Compile only (don't link).

```makefile
# Build static library (bonus)
$(LIBRARY): $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $^
```
-   **Library rule**: Creates the static library from object files.
-   **`$^`**: All dependencies (`cipher_lib.o`).

### Phony Targets

```makefile
# Clean build artifacts
clean:
	rm -f $(TARGETS) $(LIBRARY) *.o

# Clean and rebuild
rebuild: clean all

.PHONY: all clean rebuild library
```
-   **`clean`**: Removes all generated files.
-   **`rebuild`**: Cleans and then builds everything.
-   **`.PHONY`**: Declares that these targets are not actual files (prevents conflicts with files named `clean`, etc.).

### How to Use

| Command | Action |
|---------|--------|
| `make` or `make all` | Build all programs |
| `make cipher` | Build only the cipher program |
| `make clean` | Remove all generated files |
| `make rebuild` | Clean and rebuild everything |
| `make library` | Build only the static library |

---

## 7. The Bash Toolbox

The project includes a set of Bash scripts to manage a workspace for analyzing encrypted archives. These scripts automate the process of importing, listing, and inspecting archives.

### 7.1 Initialization: `init-toolbox.sh`

#### Goal
Initializes the `.sh-toolbox` directory and the `archives` tracking file. This script ensures the environment is ready for use.

#### Usage
```bash
./init-toolbox.sh
```

#### Code Explanation
The script performs the following checks and actions:
1.  **Directory Creation**: Checks if `.sh-toolbox` exists. If not, it creates it.
2.  **Foreign File Check**: Ensures the directory contains only the `archives` file. If other files exist, it exits with error code 12.
3.  **Archives File Creation**: Checks if `archives` file exists. If not, it creates it with an initial count of `0`.

**Return Codes:**
-   `10`: Success (directory/file created or already existed correctly).
-   `11`: Creation failed.
-   `12`: Directory contains unauthorized files.

### 7.2 Archive Import: `import-archive.sh`

#### Goal
Imports a `.gz` archive into the `.sh-toolbox` directory and updates the tracking file.

#### Usage
```bash
./import-archive.sh [-f] <archive_path> [archive_path2 ...]
```
-   `-f`: Force overwrite if the archive already exists in the toolbox.

#### Code Explanation
1.  **Argument Parsing**: Handles the `-f` flag and collects archive paths.
2.  **Existence Check**: Verifies that the source archive exists.
3.  **Overwrite Protection**: If the file exists in the toolbox, it asks for confirmation unless `-f` is used.
4.  **Copy**: Copies the file to `.sh-toolbox`.
5.  **Tracking Update**: Updates `.sh-toolbox/archives` by appending the filename.

**Return Codes:**
-   `0`: Success (or cancelled by user).
-   `1`: Toolbox directory missing.
-   `2`: Source archive missing or no arguments.
-   `3`: Copy failed.
-   `4`: Archives file missing.

### 7.3 Listing Archives: `ls-toolbox.sh`

#### Goal
Lists all archives currently tracked in the toolbox, displaying their status.

#### Usage
```bash
./ls-toolbox.sh
```

#### Code Explanation
1.  **Environment Check**: Verifies `.sh-toolbox` and `archives` file exist.
2.  **Parsing**: Reads `archives` file line by line.
3.  **Display**: Prints the filename, import date, and key status (known/unknown).
4.  **Consistency Check (Bonus)**:
    -   Warns if a tracked archive is missing from the directory.
    -   Warns if an untracked archive exists in the directory.

**Return Codes:**
-   `0`: Success.
-   `1`: Toolbox directory missing.
-   `2`: Archives file missing.
-   `3`: Inconsistency detected (missing or untracked files).

### 7.4 Archive Analysis: `check-archive.sh`

#### Goal
Analyzes a specific archive to identify the time of attack and which files were modified.

#### Usage
```bash
./check-archive.sh
```

#### Code Explanation
1.  **Selection**: Lists available archives and prompts the user to select one.
2.  **Decompression**: Extracts the selected archive to a temporary directory.
3.  **Log Analysis**:
    -   Parses `var/log/auth.log`.
    -   Finds the last successful login of the user `admin`.
    -   Extracts the timestamp of this event (the "attack time").
4.  **File Forensics**:
    -   Scans the `data/` directory.
    -   Identifies files modified *after* the attack timestamp.
    -   (Bonus) Identifies potential original files (same name/size, not modified).

**Return Codes:**
-   `0`: Success.
-   `1`: Toolbox directory missing.
-   `2`: Archives file missing.
-   `3`: Archive missing or decompression failed.
-   `4`: Log file missing or admin login not found.
-   `5`: Data directory missing or empty.

---

## 8. Program Usage Summary

### C Programs (Encryption/Decryption)
1.  **Compile**: `make all`
2.  **Encrypt**: `./cipher <key> <file>`
3.  **Decrypt**: `./decipher <key> <file>`
4.  **Find Key**: `./findkey <plaintext_file> <encrypted_file>`

### Bash Scripts (Toolbox & Forensics)
1.  **Initialize**: `./init-toolbox.sh`
2.  **Import**: `./import-archive.sh <archive.gz>`
3.  **List**: `./ls-toolbox.sh`
4.  **Analyze**: `./check-archive.sh`
