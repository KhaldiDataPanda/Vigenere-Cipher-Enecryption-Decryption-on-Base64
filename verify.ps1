$ErrorActionPreference = "Stop"

# Create secret file
"Hello World" | Set-Content -NoNewline secret.txt

# Encode to Base64
# certutil adds headers, we need to strip them.
# PowerShell has built-in Base64 support.
$bytes = [System.IO.File]::ReadAllBytes("$PWD/secret.txt")
$b64 = [Convert]::ToBase64String($bytes)
$b64 | Set-Content -NoNewline secret.b64

# Copy for later comparison
Copy-Item secret.b64 secret.b64.orig

# Encrypt
Write-Host "Encrypting..."
./cipher MYKEY secret.b64

# Check if file changed (it should be binary now)
$cipherBytes = [System.IO.File]::ReadAllBytes("$PWD/secret.b64")
if ($cipherBytes.Length -eq 0) { Write-Error "Cipher output is empty" }

# Find Key
Write-Host "Finding Key..."
# findkey expects plaintext (secret.txt) and ciphertext (secret.b64 which is now binary)
$keyOutput = ./findkey secret.txt secret.b64
Write-Host "Key Found: $keyOutput"

# Decrypt
# First, encode the binary cipher back to Base64 for decipher tool
$cipherB64 = [Convert]::ToBase64String($cipherBytes)
$cipherB64 | Set-Content -NoNewline secret.cipher.b64

Write-Host "Decrypting..."
./decipher MYKEY secret.cipher.b64

# Compare
$decryptedB64 = Get-Content secret.cipher.b64
$originalB64 = Get-Content secret.b64.orig

if ($decryptedB64 -eq $originalB64) {
    Write-Host "SUCCESS: Decryption matches original." -ForegroundColor Green
} else {
    Write-Host "FAILURE: Decryption does not match." -ForegroundColor Red
    Write-Host "Original: $originalB64"
    Write-Host "Decrypted: $decryptedB64"
    exit 1
}
