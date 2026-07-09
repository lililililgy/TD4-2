# Stop execution on error
$ErrorActionPreference = "Stop"

# Determine output directory relative to script path
$OutputDir = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($PSScriptRoot, "..\..\Project\Packages\Scripts"))

# Check output directory exists
if (-not (Test-Path -Path $OutputDir)) {
    Write-Error "ERROR: Output folder does not exist: $OutputDir"
    exit 2
}

# Original filenames
$OriginalDll = "CSharpLibrary.dll"
$OriginalPdb = "CSharpLibrary.pdb"

# Get timestamp (yyyyMMdd_HHmmss format)
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# Target filenames
$RenamedDll = "CSharpLibrary_${Timestamp}.dll"
$RenamedPdb1 = "CSharpLibrary_${Timestamp}.pdb"
$RenamedPdb2 = "CSharpLibrary_${Timestamp}.dll.pdb"

# Build full paths
$SourceDllPath = Join-Path $OutputDir $OriginalDll
$DestDllPath   = Join-Path $OutputDir $RenamedDll
$SourcePdbPath = Join-Path $OutputDir $OriginalPdb
$DestPdbPath1  = Join-Path $OutputDir $RenamedPdb1
$DestPdbPath2  = Join-Path $OutputDir $RenamedPdb2

Write-Host "Attempting to copy from '$SourceDllPath' to '$DestDllPath'"

# Copy DLL
try {
    Copy-Item -Path $SourceDllPath -Destination $DestDllPath -Force
} catch {
    Write-Error "ERROR: Failed to copy DLL: $_"
    exit 3
}

# Copy PDB (if exists)
if (Test-Path -Path $SourcePdbPath) {
    Write-Host "Copying PDB..."
    try {
        Copy-Item -Path $SourcePdbPath -Destination $DestPdbPath1 -Force
        Copy-Item -Path $SourcePdbPath -Destination $DestPdbPath2 -Force
    } catch {
        Write-Warning "Failed to copy PDB (continuing): $_"
    }
} else {
    Write-Host "Notice: PDB file not found: $SourcePdbPath"
}

exit 0