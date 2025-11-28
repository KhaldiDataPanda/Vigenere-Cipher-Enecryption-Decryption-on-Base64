#!/bin/bash

# import-archive.sh
# Imports archives into the .sh-toolbox environment

TOOLBOX_DIR=".sh-toolbox"
ARCHIVES_FILE="$TOOLBOX_DIR/archives"

# Check if toolbox exists
if [ ! -d "$TOOLBOX_DIR" ]; then
    echo "Error: Directory $TOOLBOX_DIR does not exist."
    exit 1
fi

FORCE=0
ARCHIVES=()

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -f)
            FORCE=1
            shift
            ;;
        *)
            ARCHIVES+=("$1")
            shift
            ;;
    esac
done

if [ ${#ARCHIVES[@]} -eq 0 ]; then
    echo "Usage: $0 [-f] <archive1> [archive2 ...]"
    exit 2
fi

for archive_path in "${ARCHIVES[@]}"; do
    if [ ! -f "$archive_path" ]; then
        echo "Error: Archive $archive_path does not exist."
        exit 2
    fi

    filename=$(basename "$archive_path")
    dest_path="$TOOLBOX_DIR/$filename"
    
    # Check if file exists in toolbox
    if [ -f "$dest_path" ]; then
        if [ "$FORCE" -eq 0 ]; then
            read -p "File $filename already exists. Overwrite? (y/n) " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                echo "Skipping $filename."
                continue # Return 0 for this file effectively (as per spec "0 si la copie a été annulée")
            fi
        fi
        # If force or user said yes, we overwrite.
    fi

    # Copy file
    cp "$archive_path" "$dest_path"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to copy $archive_path."
        exit 3
    fi

    # Update archives file
    # We need to check if the file is already listed in archives file to avoid duplicates or just append?
    # Spec says: "La valeur dans le fichier archives doit être incrémentée et une nouvelle ligne doit être ajoutée"
    # It doesn't explicitly say we should check if it's already there, but it makes sense to not duplicate entries for the same file if we overwrite it.
    # However, for simplicity and strict adherence to "increment and add line", I will just append. 
    # BUT, if we overwrite, maybe we should update the existing entry?
    # Let's look at the format: "client1-20250411-1311.gz:20251104-131504:"
    # If I overwrite, the date might change.
    # Let's check if the filename is already in the archives file.
    
    if [ ! -f "$ARCHIVES_FILE" ]; then
         echo "Error: $ARCHIVES_FILE missing."
         exit 4
    fi

    # Get current count
    count=$(head -n 1 "$ARCHIVES_FILE")
    
    # Check if entry exists
    if grep -q "^$filename:" "$ARCHIVES_FILE"; then
        # Entry exists. The spec is a bit vague on update. 
        # "Met à jour le fichier archives si besoin"
        # If we overwrite, we probably should update the timestamp.
        # Let's remove the old line and add a new one, and NOT increment the count (since it's the same file).
        
        # Remove old line
        grep -v "^$filename:" "$ARCHIVES_FILE" > "${ARCHIVES_FILE}.tmp" && mv "${ARCHIVES_FILE}.tmp" "$ARCHIVES_FILE"
        
        # We removed the line, but we kept the count line (head -n 1). 
        # Since we are re-adding it, the count of *files* is the same.
        # So we don't increment count.
        
        # Wait, if I use grep -v, I might remove the first line if it matches? No, first line is a number.
        # But grep -v might fail if file is empty or only has one line?
        # Let's be careful.
        
        # Actually, simpler approach:
        # 1. Read count
        # 2. Read all other lines
        # 3. Filter out lines starting with filename
        # 4. Write count
        # 5. Write filtered lines
        # 6. Append new line
        
        # But wait, if I remove the line, I should NOT increment the count?
        # Or does the count represent the number of lines?
        # "La valeur dans le fichier archives doit être incrémentée" implies count = number of entries.
        # So if I replace, count stays same.
        
        # Let's do this:
        # Construct the new line
        timestamp=$(date +"%Y%m%d-%H%M%S")
        new_entry="$filename:$timestamp:"
        
        # Create temp file
        temp_file=$(mktemp)
        
        # Read first line (count)
        read -r current_count < "$ARCHIVES_FILE"
        
        # Check if file is already in archives (excluding first line)
        if tail -n +2 "$ARCHIVES_FILE" | grep -q "^$filename:"; then
            # It exists, so we don't increment count.
            echo "$current_count" > "$temp_file"
            tail -n +2 "$ARCHIVES_FILE" | grep -v "^$filename:" >> "$temp_file"
            echo "$new_entry" >> "$temp_file"
        else
            # It doesn't exist, increment count
            new_count=$((current_count + 1))
            echo "$new_count" > "$temp_file"
            tail -n +2 "$ARCHIVES_FILE" >> "$temp_file"
            echo "$new_entry" >> "$temp_file"
        fi
        
        mv "$temp_file" "$ARCHIVES_FILE"
        
    else
        # File not in archives (e.g. manual copy or first time)
        # Just append
        timestamp=$(date +"%Y%m%d-%H%M%S")
        new_entry="$filename:$timestamp:"
        
        # Read count
        read -r current_count < "$ARCHIVES_FILE"
        new_count=$((current_count + 1))
        
        # Update file
        # We need to use a temp file to safely update head
        temp_file=$(mktemp)
        echo "$new_count" > "$temp_file"
        tail -n +2 "$ARCHIVES_FILE" >> "$temp_file"
        echo "$new_entry" >> "$temp_file"
        mv "$temp_file" "$ARCHIVES_FILE"
    fi
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to update archives file."
        exit 4
    fi
    
    echo "Imported $filename."

done

exit 0
