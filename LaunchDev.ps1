param (
    [switch]$Generate
)

$ErrorActionPreference = "Stop"

# Repository root directory
$RootDir = $PSScriptRoot

Write-Host "--- Launching Development Environment ---" -ForegroundColor Cyan

# 1. C++ Solution Setup and Launch
$slnPath = Join-Path $RootDir "Project\ONEngine.sln"

if ($Generate -or -not (Test-Path $slnPath)) {
    Write-Host "C++ solution file not found or force generation requested. Running GenerateProject..." -ForegroundColor Yellow
    Push-Location -Path (Join-Path $RootDir "Project")
    try {
        powershell -NoProfile -ExecutionPolicy Bypass -File .\GenerateProject.ps1
    } finally {
        Pop-Location
    }
}

if (Test-Path $slnPath) {
    Write-Host "Opening C++ Solution: $slnPath" -ForegroundColor Green
    Start-Process $slnPath
} else {
    Write-Warning "Failed to locate or generate C++ solution file."
}

# 2. C# Project Launch in VS Code
$csharpFolder = Join-Path $RootDir "SubProjects\CSharpLibrary"

if (Test-Path $csharpFolder) {
    Write-Host "Opening C# Folder in VS Code: $csharpFolder" -ForegroundColor Green
    
    $codeCmd = "code"
    $codeFound = $false
    
    # Check if 'code' is in PATH
    if (Get-Command $codeCmd -ErrorAction SilentlyContinue) {
        $codeFound = $true
    } else {
        # Check standard installation locations
        $pathsToSearch = @(
            "$env:LocalAppData\Programs\Microsoft VS Code\bin\code.cmd",
            "$env:ProgramFiles\Microsoft VS Code\bin\code.cmd",
            "$env:ProgramFiles(x86)\Microsoft VS Code\bin\code.cmd",
            "C:\Program Files\Microsoft VS Code\bin\code.cmd"
        )
        foreach ($path in $pathsToSearch) {
            if (Test-Path $path) {
                $codeCmd = $path
                $codeFound = $true
                break
            }
        }
    }
    
    if ($codeFound) {
        Start-Process $codeCmd -ArgumentList "`"$csharpFolder`""
    } else {
        Write-Warning "VS Code ('code' command) not found in PATH or standard installation locations. Please open the C# folder manually."
    }
} else {
    Write-Warning "C# library folder not found: $csharpFolder"
}

Write-Host "Launch process completed." -ForegroundColor Cyan
