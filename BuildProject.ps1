param (
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"

# 1. Locate MSBuild.exe
Write-Host "Locating MSBuild.exe..." -ForegroundColor Cyan
$vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswherePath)) {
    Write-Error "vswhere.exe not found. Visual Studio installation is required."
    exit 1
}

$msbuildPath = & $vswherePath -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
if (-not $msbuildPath) {
    Write-Error "MSBuild.exe not found."
    exit 1
}
Write-Host "Found MSBuild: $msbuildPath" -ForegroundColor Green

# 2. Generate and Build C++ project (ONEngine)
Write-Host "--- Preparing C++ (ONEngine) Project Build ---" -ForegroundColor Cyan
Push-Location -Path "Project"
try {
    # Generate project files
    Write-Host "Generating C++ project files using Premake..."
    & .\premake5.exe vs2022
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to generate C++ project files."
    }

    # Run build
    Write-Host "Building ONEngine.sln using MSBuild ($Configuration)..."
    & $msbuildPath ONEngine.sln /t:Build /p:Configuration=$Configuration /p:Platform=x64 /m
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to build C++ project."
    }
} finally {
    Pop-Location
}

# 3. Generate and Build C# project (CSharpLibrary)
Write-Host "--- Preparing C# (CSharpLibrary) Project Build ---" -ForegroundColor Cyan
Push-Location -Path "SubProjects/CSharpLibrary"
try {
    # Generate project files
    Write-Host "Generating C# project files using Premake..."
    $premakeExe = "../../Project/premake5.exe"
    & $premakeExe vs2022
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to generate C# project files."
    }

    # Run build
    Write-Host "Building CSharpLibrary.csproj using dotnet build ($Configuration)..."
    dotnet build CSharpLibrary.csproj --configuration $Configuration
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to build C# project."
    }
} finally {
    Pop-Location
}

Write-Host "All projects built successfully." -ForegroundColor Green
