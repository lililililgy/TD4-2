# プロジェクトのルートをスクリプトの位置に固定
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

# premake5.exe が同じフォルダにある前提
Write-Host "Running Premake..."
& ".\premake5.exe" $VSVersion

if ($LASTEXITCODE -eq 0) {
    Write-Host "Premake finished. Opening ONEngine.sln..."
    Start-Process -FilePath "ONEngine.sln"
} else {
    Write-Host "Premake failed with exit code $LASTEXITCODE"
}