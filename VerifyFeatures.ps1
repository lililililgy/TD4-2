# VerifyFeatures.ps1
# ONEngine Core Features Verification Tool (SerializeField & Hot Reload)

$ErrorActionPreference = "Stop"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "     ONEngine Core Feature Verifier      " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# --- 1. [Auto] SerializeField Verification ---
Write-Host "`n[1/2] Verifying SerializeField (Auto)..." -ForegroundColor Yellow

$EngineExe = "$PSScriptRoot/Project/Engine/Bin/Debug/ONEngine.exe"
if (-not (Test-Path $EngineExe)) {
    $EngineExe = "$PSScriptRoot/Project/Engine/Bin/Development/ONEngine.exe"
}

if (-not (Test-Path $EngineExe)) {
    Write-Warning "ONEngine.exe not found. Please build the engine first."
    exit 1
}

# Run test scene
$TestOutput = "$PSScriptRoot/Generated/verify_serialize_field.json"
if (Test-Path $TestOutput) { Remove-Item $TestOutput }

Write-Host "Running test scene..."
& $EngineExe --test-mode --test-duration 120 --test-output $TestOutput --test-scene InheritSerializeFieldTest --test-input "$PSScriptRoot/Project/Assets/Tests/InheritSerializeField_test.json" | Out-Null

if (Test-Path $TestOutput) {
    $Result = Get-Content $TestOutput | ConvertFrom-Json
    if ($Result.result -eq "passed") {
        Write-Host "[PASSED] SerializeField inheritance & deserialization is working properly." -ForegroundColor Green
    } else {
        Write-Error "[FAILED] SerializeField test scene failed."
    }
    Remove-Item $TestOutput
} else {
    Write-Error "[ERROR] Test result file was not generated."
}

# --- 2. [Manual/Auto] Hot Reload Verification ---
Write-Host "`n[2/2] Verifying Hot Reload (Manual/Auto)..." -ForegroundColor Yellow

# Target file path
$TargetFile = "$PSScriptRoot/SubProjects/CSharpLibrary/Scripts/Test/ProfileTest.cs"
if (-not (Test-Path $TargetFile)) {
    Write-Error "Target file not found: $TargetFile"
}

# Create backup
$BackupFile = "${TargetFile}.bak"
Copy-Item $TargetFile $BackupFile

# Temporarily modify C# file
$Content = Get-Content $TargetFile -Raw
$HotReloadCode = "`n`t`tDebug.Log(`"=== HOT RELOAD SUCCESS ===`");`n"
$NewContent = $Content -replace 'public override void Update\(\) \{', "public override void Update() {${HotReloadCode}"

Set-Content $TargetFile $NewContent -NoNewline

Write-Host "Temporary check code injected successfully." -ForegroundColor Green
Write-Host "Instructions:" -ForegroundColor Cyan
Write-Host "1. Start ONEngine.exe (Debug or Development configuration)." -ForegroundColor Cyan
Write-Host "2. Once running, press [Enter] key in this window to build." -ForegroundColor Cyan
Read-Host

Write-Host "Building C# library (triggering Hot Reload)..."
Push-Location "$PSScriptRoot/SubProjects/CSharpLibrary"
dotnet build CSharpLibrary.csproj --configuration Debug /p:Platform=x64 | Out-Null
Pop-Location

Write-Host "`nBuild complete. New assembly transfered." -ForegroundColor Green
Write-Host "Please verify:" -ForegroundColor Cyan
Write-Host "Check the console output in the running ONEngine." -ForegroundColor Cyan
Write-Host "If you see: '=== HOT RELOAD SUCCESS ===' being logged continuously," -ForegroundColor Green
Write-Host "Hot Reload is working perfectly." -ForegroundColor Cyan
Write-Host "Press [Enter] key in this window to finish and clean up."
Read-Host

# Restore code
Copy-Item $BackupFile $TargetFile -Force
Remove-Item $BackupFile

Write-Host "Temporary code reverted." -ForegroundColor Green
Write-Host "`nAll verifications completed successfully!" -ForegroundColor Cyan
