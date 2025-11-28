#!/bin/bash

# check-archive.sh
# Analyzes an archive to find attack details

TOOLBOX_DIR=".sh-toolbox"
ARCHIVES_FILE="$TOOLBOX_DIR/archives"

# 1. Check environment
if [ ! -d "$TOOLBOX_DIR" ]; then
    echo "Error: Directory $TOOLBOX_DIR does not exist."
    exit 1
fi

if [ ! -f "$ARCHIVES_FILE" ]; then
    echo "Error: File $ARCHIVES_FILE does not exist."
    exit 2
fi

# 2. List archives and ask for selection
echo "Available archives:"
mapfile -t lines < "$ARCHIVES_FILE"
archives=()

# Skip first line
for ((i=1; i<${#lines[@]}; i++)); do
    line="${lines[$i]}"
    IFS=':' read -r filename date key <<< "$line"
    echo "$i. $filename"
    archives+=("$filename")
done

if [ ${#archives[@]} -eq 0 ]; then
    echo "No archives found."
    exit 0
fi

read -p "Select an archive (1-${#archives[@]}): " selection

if [[ ! "$selection" =~ ^[0-9]+$ ]] || [ "$selection" -lt 1 ] || [ "$selection" -gt "${#archives[@]}" ]; then
    echo "Invalid selection."
    exit 0
fi

selected_archive="${archives[$((selection-1))]}"
archive_path="$TOOLBOX_DIR/$selected_archive"

if [ ! -f "$archive_path" ]; then
    echo "Error: Archive file not found."
    exit 3
fi

# 3. Decompress to temp
temp_dir=$(mktemp -d)
echo "Decompressing to $temp_dir..."

# Check if it's a valid gz file
if ! gzip -t "$archive_path" 2>/dev/null; then
     echo "Error: Invalid gzip file."
     rm -rf "$temp_dir"
     exit 3
fi

# Decompress
# We use tar if it's a tar.gz, or just gunzip if it's a single file?
# The spec says "archive au format .gz". Usually implies a single file or a tarball.
# "Chaque archive contient les éléments suivants : un fichier ... plusieurs dossiers ..."
# This implies it's a tarball (tar.gz).
tar -xzf "$archive_path" -C "$temp_dir"
if [ $? -ne 0 ]; then
    echo "Error: Decompression failed."
    rm -rf "$temp_dir"
    exit 3
fi

# 4. Parse logs
log_file="$temp_dir/var/log/auth.log"
if [ ! -f "$log_file" ]; then
    echo "Error: Log file missing."
    rm -rf "$temp_dir"
    exit 4
fi

# Find last connection of "admin"
# Example log format: "Nov  4 13:15:04 server sshd[123]: Accepted password for admin from ..."
# We need to parse the date.
# "Affiche la date et l’heure de la dernière connexion de l’utilisateur « admin »"

# Grep for "Accepted password for admin" or similar successful login
# Let's assume standard auth.log format.
last_login_line=$(grep "Accepted password for admin" "$log_file" | tail -n 1)

if [ -z "$last_login_line" ]; then
    # Maybe public key?
    last_login_line=$(grep "Accepted publickey for admin" "$log_file" | tail -n 1)
fi

if [ -z "$last_login_line" ]; then
    echo "Could not find admin login."
    # Continue? Or exit? Spec says "Affiche la date..."
    # If we can't find it, we can't determine attack time.
    rm -rf "$temp_dir"
    exit 4 # Treat as missing info
fi

echo "Last admin login found: $last_login_line"

# Extract date/time
# Format: Month Day Time
# e.g. Nov  4 13:15:04
# We need to convert this to a timestamp for comparison.
# Since year is missing in syslog usually, we might need to guess or assume current year?
# Or maybe the archive name has the year? "client1-20250411..."
# Let's assume the year from the archive filename or just current year if not available.
# But wait, `date` command can handle "Nov 4 13:15:04". It assumes current year.
# If the attack was in 2025 (as per filename example), and we are in 2024, `date` might get it wrong if we don't specify year.
# However, for comparison, if all files are in the same year, it doesn't matter much, UNLESS we cross year boundary.
# Let's try to extract year from archive name if possible, or just rely on `date` smarts.
# Example: client1-20250411-1311.gz -> 2025.

year=$(echo "$selected_archive" | grep -oP '\d{4}' | head -n 1)
if [ -z "$year" ]; then
    year=$(date +%Y)
fi

# Extract date string from log line
# "Nov  4 13:15:04" is the first 15 chars usually.
log_date_str=$(echo "$last_login_line" | awk '{print $1, $2, $3}')
# Combine with year
full_date_str="$log_date_str $year"

# Convert to timestamp
attack_timestamp=$(date -d "$full_date_str" +%s)
echo "Attack timestamp: $attack_timestamp ($(date -d @$attack_timestamp))"

# 5. List modified files
data_dir="$temp_dir/data"
if [ ! -d "$data_dir" ]; then
    echo "Error: Data directory missing."
    rm -rf "$temp_dir"
    exit 5
fi

# Check if empty
if [ -z "$(ls -A "$data_dir")" ]; then
    echo "Error: Data directory is empty."
    rm -rf "$temp_dir"
    exit 5
fi

echo "Files modified after attack:"
# Find files in data_dir
# We need to check modification time of each file.
find "$data_dir" -type f | while read -r file; do
    file_mtime=$(stat -c %Y "$file")
    if [ "$file_mtime" -gt "$attack_timestamp" ]; then
        echo "$(basename "$file")"
    fi
done

# [BONUS] List non-modified files with same name and size
# "Parcourt les données puis affiche la liste des fichiers non modifiés qui portent le même nom et ont la même taille que les fichiers modifiés"
# This implies we compare modified files against... what?
# "Par chance, l’utilisateur « admin » n’est pas « root » : certains fichiers n’ont pas été modifiés pendant l’attaque."
# "Pour les identifier, il faut rechercher les fichiers en lecture seule qui n’ont pas été modifiés après la dernière connexion de l’utilisateur « admin »."
# "nous supposerons que deux fichiers avec le même nom et la même taille sont identiques."
# This phrasing is a bit ambiguous.
# Does it mean we have duplicates in the data folder?
# Or does it mean we should look for files that *should* be there?
# "Parcourt les données puis affiche la liste des fichiers non modifiés qui portent le même nom et ont la même taille que les fichiers modifiés"
# Maybe there are backup files?
# Let's skip this bonus for now to ensure core functionality, or try to interpret it.
# If I have file A (modified) and file B (not modified), and A and B have same name? They can't be in same folder.
# Maybe in subfolders?
# Let's iterate all files again.

echo "Potential original files (same name/size, not modified):"
# Store modified files info: name -> size
declare -A modified_files

# First pass: identify modified files
while IFS= read -r file; do
    file_mtime=$(stat -c %Y "$file")
    if [ "$file_mtime" -gt "$attack_timestamp" ]; then
        name=$(basename "$file")
        size=$(stat -c %s "$file")
        modified_files["$name"]=$size
    fi
done < <(find "$data_dir" -type f)

# Second pass: find non-modified matching files
while IFS= read -r file; do
    file_mtime=$(stat -c %Y "$file")
    if [ "$file_mtime" -le "$attack_timestamp" ]; then
        name=$(basename "$file")
        size=$(stat -c %s "$file")
        
        # Check if this file matches a modified file
        if [ -n "${modified_files[$name]}" ]; then
            if [ "${modified_files[$name]}" -eq "$size" ]; then
                echo "$name"
            fi
        fi
    fi
done < <(find "$data_dir" -type f)

# Cleanup
rm -rf "$temp_dir"
exit 0
