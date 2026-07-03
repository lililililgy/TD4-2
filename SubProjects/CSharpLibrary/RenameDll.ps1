# エラー発生時に処理を停止する設定
$ErrorActionPreference = "Stop"

# --- スクリプトの配置場所を基準に出力フォルダを決定 ---
# $PSScriptRoot はこのスクリプトが存在するフォルダのパスです
$OutputDir = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($PSScriptRoot, "..\..\Project\Packages\Scripts"))

# 出力フォルダが存在するかチェック
if (-not (Test-Path -Path $OutputDir)) {
    Write-Error "ERROR: Output folder does not exist: $OutputDir"
    exit 2
}

# オリジナルのファイル名
$OriginalDll = "CSharpLibrary.dll"
$OriginalPdb = "CSharpLibrary.pdb"

# --- タイムスタンプの取得 (yyyyMMdd_HHmmss 形式) ---
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# リネーム後のファイル名
$RenamedDll = "CSharpLibrary_${Timestamp}.dll"
$RenamedPdb = "CSharpLibrary_${Timestamp}.pdb"

# フルパスの組み立て
$SourceDllPath = Join-Path $OutputDir $OriginalDll
$DestDllPath   = Join-Path $OutputDir $RenamedDll
$SourcePdbPath = Join-Path $OutputDir $OriginalPdb
$DestPdbPath   = Join-Path $OutputDir $RenamedPdb

Write-Host "Attempting to copy from '$SourceDllPath' to '$DestDllPath'"

# DLL のコピー
try {
    Copy-Item -Path $SourceDllPath -Destination $DestDllPath -Force
} catch {
    Write-Error "ERROR: Failed to copy DLL: $_"
    exit 3
}

# PDB のコピー（存在する場合のみ）
if (Test-Path -Path $SourcePdbPath) {
    Write-Host "Copying PDB..."
    try {
        Copy-Item -Path $SourcePdbPath -Destination $DestPdbPath -Force
    } catch {
        Write-Warning "Failed to copy PDB (continuing): $_"
    }
} else {
    Write-Host "Notice: PDB file not found: $SourcePdbPath"
}

exit 0