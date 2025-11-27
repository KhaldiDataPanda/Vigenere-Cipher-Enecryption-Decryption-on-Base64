# Clean workspace script
# Removes compiled executables, object files, and library files

Write-Host "Cleaning workspace..." -ForegroundColor Cyan

# Files to remove
$filesToRemove = @(
    "*.exe",      # Executables
    "*.o",        # Object files
    "*.a",        # Static libraries
    "*.obj",       # Windows object files
    "*.orig",     # Backup files
    "*.b64"       # Base64 encoded files
)

$removedCount = 0

foreach ($pattern in $filesToRemove) {
    $files = Get-ChildItem -Path $PSScriptRoot -Filter $pattern -ErrorAction SilentlyContinue
    foreach ($file in $files) {
        Remove-Item $file.FullName -Force
        Write-Host "  Removed: $($file.Name)" -ForegroundColor Yellow
        $removedCount++
    }
}

if ($removedCount -eq 0) {
    Write-Host "Workspace is already clean." -ForegroundColor Green
} else {
    Write-Host "Cleanup complete. Removed $removedCount file(s)." -ForegroundColor Green
}
