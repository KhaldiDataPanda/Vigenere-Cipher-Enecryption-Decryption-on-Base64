# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
AR = ar
ARFLAGS = rcs

# Targets
TARGETS = cipher decipher findkey
LIBRARY = libcipher.a

# Source files
CIPHER_SRC = cipher.c
DECIPHER_SRC = decipher.c
FINDKEY_SRC = findkey.c

# Library source files (for bonus)
LIB_SRC = cipher_lib.c
LIB_HDR = cipher_lib.h

# Object files
CIPHER_OBJ = $(CIPHER_SRC:.c=.o)
DECIPHER_OBJ = $(DECIPHER_SRC:.c=.o)
FINDKEY_OBJ = $(FINDKEY_SRC:.c=.o)
LIB_OBJ = $(LIB_SRC:.c=.o)

# Default target: build all programs
all: $(TARGETS)

# Build cipher program (linked with library)
cipher: $(CIPHER_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(CIPHER_OBJ) -L. -lcipher

# Build decipher program (linked with library)
decipher: $(DECIPHER_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(DECIPHER_OBJ) -L. -lcipher

# Build findkey program (linked with library)
findkey: $(FINDKEY_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(FINDKEY_OBJ) -L. -lcipher

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build static library (bonus)
$(LIBRARY): $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $^

# Build library target
library: $(LIBRARY)

# Clean build artifacts
clean:
	rm -f $(TARGETS) $(LIBRARY) *.o

# Clean and rebuild
rebuild: clean all

.PHONY: all clean rebuild library
