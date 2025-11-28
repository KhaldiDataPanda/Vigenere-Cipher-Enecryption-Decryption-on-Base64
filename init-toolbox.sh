#!/bin/bash

# init-toolbox.sh
# Initializes the .sh-toolbox environment

TOOLBOX_DIR=".sh-toolbox"
ARCHIVES_FILE="$TOOLBOX_DIR/archives"

# Function to check if directory contains only allowed files
check_dir_content() {
    local dir="$1"
    local allowed_file="archives"
    
    # Count files/dirs in the directory
    local count=$(ls -A "$dir" | wc -l)
    
    if [ "$count" -eq 0 ]; then
        return 0 # Empty is fine (we'll create archives)
    fi
    
    # Check if only archives file exists
    if [ "$count" -eq 1 ] && [ -f "$ARCHIVES_FILE" ]; then
        return 0
    fi
    
    return 1
}

# 1. Check existence of .sh-toolbox
if [ ! -d "$TOOLBOX_DIR" ]; then
    mkdir "$TOOLBOX_DIR"
    if [ $? -ne 0 ]; then
        echo "Error: Could not create directory $TOOLBOX_DIR"
        exit 11
    fi
    echo "Directory $TOOLBOX_DIR created."
    created_dir=1
else
    created_dir=0
fi

# Check for foreign files before creating archives if it doesn't exist
# If directory existed, we need to check its content
if [ "$created_dir" -eq 0 ]; then
     # If archives file exists, check if there are other files
    if [ -f "$ARCHIVES_FILE" ]; then
         if ! check_dir_content "$TOOLBOX_DIR"; then
            echo "Error: Directory $TOOLBOX_DIR contains other files."
            exit 12
        fi
    else
        # If archives file doesn't exist, directory should be empty
        if [ "$(ls -A "$TOOLBOX_DIR" | wc -l)" -ne 0 ]; then
             echo "Error: Directory $TOOLBOX_DIR contains other files."
             exit 12
        fi
    fi
fi

# 3. Check existence of archives file
if [ ! -f "$ARCHIVES_FILE" ]; then
    echo "0" > "$ARCHIVES_FILE"
    if [ $? -ne 0 ]; then
        echo "Error: Could not create file $ARCHIVES_FILE"
        exit 11
    fi
    echo "File $ARCHIVES_FILE created."
    created_file=1
else
    created_file=0
fi

# Return codes
if [ "$created_dir" -eq 1 ] || [ "$created_file" -eq 1 ]; then
    exit 10
elif [ "$created_dir" -eq 0 ] && [ "$created_file" -eq 0 ]; then
    exit 10
fi

exit 11 # Should not be reached if logic is correct
