#!/bin/bash

# ls-toolbox.sh
# Lists archives in the .sh-toolbox environment

TOOLBOX_DIR=".sh-toolbox"
ARCHIVES_FILE="$TOOLBOX_DIR/archives"

# 2. Check if toolbox exists
if [ ! -d "$TOOLBOX_DIR" ]; then
    echo "Error: Directory $TOOLBOX_DIR does not exist."
    exit 1
fi

# 3. Check if archives file exists
if [ ! -f "$ARCHIVES_FILE" ]; then
    echo "Error: File $ARCHIVES_FILE does not exist."
    exit 2
fi

# Read archives file line by line (skipping first line)
# Format: filename:date:key
# We need to check for consistency errors too.

error_code=0

# Read all lines into an array to avoid subshell issues with exit codes
mapfile -t lines < "$ARCHIVES_FILE"

# Skip first line (count)
for ((i=1; i<${#lines[@]}; i++)); do
    line="${lines[$i]}"
    IFS=':' read -r filename date key <<< "$line"
    
    # Check if key is known
    key_status="unknown"
    if [ -n "$key" ]; then
        key_status="known"
    fi
    
    echo "Archive: $filename | Date: $date | Key: $key_status"
    
    # [BONUS] Check if archive exists in toolbox
    if [ ! -f "$TOOLBOX_DIR/$filename" ]; then
        echo "Error: Archive $filename mentioned in archives file but missing in directory." >&2
        error_code=3
    fi
done

# [BONUS] Check if there are archives in directory not in archives file
# Get list of .gz files in toolbox
for file in "$TOOLBOX_DIR"/*.gz; do
    # Check if glob matched anything
    if [ ! -e "$file" ]; then
        continue
    fi
    
    filename=$(basename "$file")
    
    # Check if filename is in archives file
    if ! grep -q "^$filename:" "$ARCHIVES_FILE"; then
        echo "Warning: Archive $filename exists but is not listed in archives file." >&2
        if [ "$error_code" -eq 0 ]; then
            error_code=3
        fi
    fi
done

exit "$error_code"
