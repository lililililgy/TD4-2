param (
    [string]$Configuration = "Development",
    [string]$OutputPath = "dist/ONEngine_DevEnv.zip"
)

$ErrorActionPreference = "Stop"

# Project root directory
$RootDir = [System.IO.Path]::GetFullPath("$PSScriptRoot")

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "     ONEngine Package Generator          " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration"
Write-Host "Output ZIP:    $OutputPath"

# 1. Run Build
Write-Host "`n[1/5] Building projects..." -ForegroundColor Yellow

$vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswherePath)) {
    Write-Error "vswhere.exe not found. Visual Studio is required."
    exit 1
}
$msbuildPath = & $vswherePath -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
if (-not $msbuildPath) {
    Write-Error "MSBuild.exe not found."
    exit 1
}

# Build C++ (ONEngine)
Write-Host "Building C++ ONEngine ($Configuration)..."
Push-Location -Path "$RootDir/Project"
try {
    & .\premake5.exe vs2022 | Out-Null
    & $msbuildPath ONEngine.sln /t:Build /p:Configuration=$Configuration /p:Platform=x64 /m | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to build C++ project."
        exit 1
    }
} finally {
    Pop-Location
}

# Build C# (CSharpLibrary)
$CSConfig = $Configuration
if ($Configuration -eq "Development") {
    $CSConfig = "Release"
}
Write-Host "Building C# CSharpLibrary ($CSConfig)..."
Push-Location -Path "$RootDir/SubProjects/CSharpLibrary"
try {
    $premakeExe = "$RootDir/Project/premake5.exe"
    & $premakeExe vs2022 | Out-Null
    dotnet build CSharpLibrary.csproj --configuration $CSConfig | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to build C# project."
        exit 1
    }
} finally {
    Pop-Location
}

# 2. Prepare Temp Folder
Write-Host "`n[2/5] Preparing temporary folders..." -ForegroundColor Yellow
$TempDir = "$RootDir/dist/temp_package/ONEngine_DevEnv"
if (Test-Path $TempDir) {
    Remove-Item $TempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $TempDir -Force | Out-Null

$ProjDest = "$TempDir/Project"
$SubProjDest = "$TempDir/SubProjects/CSharpLibrary"

New-Item -ItemType Directory -Path $ProjDest -Force | Out-Null
New-Item -ItemType Directory -Path "$TempDir/SubProjects" -Force | Out-Null

# 3. Copy Binaries and Assets
Write-Host "`n[3/5] Copying engine binaries and assets..." -ForegroundColor Yellow

# EXE & DLLs
$ExeSourceDir = "$RootDir/Generated/Outputs/$Configuration"
$Binaries = @("ONEngine.exe", "dxcompiler.dll", "dxil.dll", "mono-2.0-sgen.dll")
foreach ($bin in $Binaries) {
    $src = "$ExeSourceDir/$bin"
    if (Test-Path $src) {
        Copy-Item $src -Destination $ProjDest -Force
        Write-Host "Copied $bin"
    } else {
        Write-Warning "Could not find $bin at $src. Skipping."
    }
}

# Assets & Packages
Write-Host "Copying Assets and Packages..."
Copy-Item "$RootDir/Project/Assets" -Destination $ProjDest -Recurse -Force
Copy-Item "$RootDir/Project/Packages" -Destination $ProjDest -Recurse -Force

# C# Source Project
Write-Host "Copying C# library files..."
$CSProjSrc = "$RootDir/SubProjects/CSharpLibrary"
Copy-Item $CSProjSrc -Destination "$TempDir/SubProjects" -Recurse -Force

# Clean up C# temp files
$ExcludeFolders = @("bin", "obj", ".vs", "packages")
foreach ($folder in $ExcludeFolders) {
    $targetPath = "$SubProjDest/$folder"
    if (Test-Path $targetPath) {
        Remove-Item $targetPath -Recurse -Force
        Write-Host "Cleaned C# temp folder: $folder"
    }
}

# 4. Copy Utility Files
Write-Host "`n[4/5] Copying utility files..." -ForegroundColor Yellow

# Launch.bat
$LaunchBatContent = "@echo off`r`ncd Project`r`nstart ONEngine.exe"
Set-Content -Path "$TempDir/Launch.bat" -Value $LaunchBatContent -Encoding Ascii

# README.txt
$ReadmeSource = "$RootDir/Docs/README_DevEnv.txt"
if (Test-Path $ReadmeSource) {
    Copy-Item $ReadmeSource -Destination "$TempDir/README.txt" -Force
    Write-Host "Copied README.txt"
} else {
    Write-Warning "README_DevEnv.txt not found. Packaging without README."
}

# 5. Compress and Cleanup
Write-Host "`n[5/5] Compiling ZIP package..." -ForegroundColor Yellow

if ([string]::IsNullOrEmpty($OutputPath)) {
    $OutputPath = "dist/ONEngine_DevEnv.zip"
}
$ZipFilePath = "$RootDir/$OutputPath"
$ZipFilePath = $ZipFilePath -replace '/', '\'
$OutputParent = Split-Path $ZipFilePath -Parent
if (-not (Test-Path $OutputParent)) {
    New-Item -ItemType Directory -Path $OutputParent -Force | Out-Null
}
if (Test-Path $ZipFilePath) {
    Remove-Item $ZipFilePath -Force
}

Compress-Archive -Path "$TempDir" -DestinationPath $ZipFilePath -Force
Write-Host "Successfully generated: $ZipFilePath" -ForegroundColor Green

Remove-Item "$RootDir/dist/temp_package" -Recurse -Force
Write-Host "Cleaned up temporary packaging folders."

Write-Host "`nPackaging process completed successfully!" -ForegroundColor Cyan
