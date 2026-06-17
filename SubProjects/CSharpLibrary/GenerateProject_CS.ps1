$PremakePath = "../../Project/premake5.exe"
if (Test-Path $PremakePath) {
    Write-Host "Generating C# Project..." -ForegroundColor Cyan
    & $PremakePath vs2022
    Write-Host "Done." -ForegroundColor Green
} else {
    Write-Error "premake5.exe not found at $PremakePath"
}
