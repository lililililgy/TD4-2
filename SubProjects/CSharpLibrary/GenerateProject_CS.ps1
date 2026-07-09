# このスクリプトが存在するディレクトリに移動
Set-Location -Path "$PSScriptRoot"

# Detect installed Visual Studio version dynamically
$VSVersion = "vs2022" # default fallback
$vswhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
if (Test-Path $vswhere) {
    $installationVersion = & $vswhere -latest -property installationVersion
    if ($installationVersion -match "^17\.") {
        $VSVersion = "vs2022"
    } elseif ($installationVersion -match "^16\.") {
        $VSVersion = "vs2019"
    } elseif ($installationVersion -match "^15\.") {
        $VSVersion = "vs2017"
    }
}

$PremakePath = "../../Project/premake5.exe"
if (Test-Path "$PremakePath") {
    Write-Host "Generating C# Project in $PSScriptRoot..." -ForegroundColor Cyan
    & "$PremakePath" $VSVersion
    Write-Host "Done." -ForegroundColor Green
} else {
    Write-Error "premake5.exe not found at $PremakePath"
}
