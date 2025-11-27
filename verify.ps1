$ErrorActionPreference = "Stop"

# Create secret file
"Hello World" | Set-Content -NoNewline secret.txt

# Encode to Base64
# PowerShell has built-in Base64 support.
$bytes = [System.IO.File]::ReadAllBytes("$PWD/secret.txt")
$b64 = [Convert]::ToBase64String($bytes)
$b64 | Set-Content -NoNewline secret.b64

# Copy for later comparison
Copy-Item secret.b64 secret.b64.orig

# Encrypt (operates on Base64 string, outputs Base64 string)
Write-Host "Encrypting..."
./cipher MYKEY secret.b64

# Check if file changed
$cipherContent = Get-Content secret.b64
if ($cipherContent.Length -eq 0) { Write-Error "Cipher output is empty" }

# Copy encrypted file for decryption
Copy-Item secret.b64 secret.cipher.b64

# Find Key (compare original Base64 plaintext with encrypted Base64)
Write-Host "Finding Key..."
$keyOutput = ./findkey secret.b64.orig secret.b64
Write-Host "Key Found: $keyOutput"

# Decrypt (operates on encrypted Base64 string, outputs original Base64 string)
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
