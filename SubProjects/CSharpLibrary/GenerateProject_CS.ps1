# このスクリプトが存在するディレクトリに移動
Set-Location -Path $PSScriptRoot

$PremakePath = "../../Project/premake5.exe"
if (Test-Path $PremakePath) {
    Write-Host "Generating C# Project in $PSScriptRoot..." -ForegroundColor Cyan
    & $PremakePath vs2022
    Write-Host "Done." -ForegroundColor Green
} else {
    Write-Error "premake5.exe not found at $PremakePath"
}
