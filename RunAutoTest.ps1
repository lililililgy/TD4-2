param (
    [string]$SceneName = "",
    [string]$InputPath = "",
    [int]$Duration = 180,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

# 1. Path check
if ($PSScriptRoot -match "[^\x00-\x7F]") {
    Write-Warning "Warning: Script path contains multi-byte characters. This might cause Mono to crash."
}

# 2. Build if required
if (-not $SkipBuild) {
    Write-Host "Building project..." -ForegroundColor Cyan
    & "$PSScriptRoot/BuildProject.ps1" -Configuration "Debug"
}

# 3. Verify EXE existence
$exePath = "$PSScriptRoot/Generated/Outputs/Debug/ONEngine.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "Error: Executable not found. Path: $exePath"
    exit 1
}

# 4. Clean up previous results
$resultPath = "$PSScriptRoot/Generated/test_results.json"
if (Test-Path $resultPath) {
    Remove-Item $resultPath -Force
}

# 5. Assemble arguments
$args = @("--test-mode", "--test-duration", $Duration, "--test-output", $resultPath)
if ($SceneName) {
    $args += @("--test-scene", $SceneName)
}
if ($InputPath) {
    $absoluteInputPath = [System.IO.Path]::GetFullPath($InputPath)
    $args += @("--test-input", $absoluteInputPath)
}

# 6. Run application in test mode
Write-Host "Starting ONEngine.exe..." -ForegroundColor Cyan
Write-Host "Args: $args"

$process = Start-Process -FilePath $exePath -ArgumentList $args -WorkingDirectory "$PSScriptRoot/Project" -PassThru -NoNewWindow

# Wait with timeout (Duration / 60 + 60 seconds)
$timeoutSeconds = [Math]::Max(60, [int]($Duration / 60) + 60)
Write-Host "Waiting for test completion (Timeout: $timeoutSeconds seconds)..."

# Wait for process exit with timeout
try {
    $process | Wait-Process -Timeout $timeoutSeconds
} catch {
    if ($_.Exception -match "TimeoutException" -or $_.Message -match "timeout") {
        Write-Error "Error: Test run timed out after $timeoutSeconds seconds. Terminating process."
    } else {
        Write-Error "Error: Exception while waiting for process: $_"
    }
    if (-not $process.HasExited) {
        $process | Stop-Process -Force
    }
    exit 1
}

# 7. Verify results
$exitCode = $process.ExitCode
Write-Host "Process exited with code $exitCode."

if ($exitCode -ne 0) {
    Write-Error "Test Failed: Application exited with error code $exitCode."
    exit $exitCode
}

if (-not (Test-Path $resultPath)) {
    Write-Error "Test Failed: Result JSON file $resultPath was not created."
    exit 1
}

$results = Get-Content $resultPath -Raw | ConvertFrom-Json
if ($results.success) {
    Write-Host "Test Passed: $($results.message) (Frames: $($results.frames))" -ForegroundColor Green
    exit 0
} else {
    Write-Error "Test Failed: $($results.message)"
    exit 1
}
