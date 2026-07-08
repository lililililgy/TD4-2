# VerifyFeatures.ps1
# ONEngine コア機能動作検証ツール (SerializeField & ホットリロード)

$ErrorActionPreference = "Stop"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "     ONEngine コア機能動作検証ツール     " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# --- 1. [自動検証] SerializeField の検証 ---
Write-Host "`n[1/2] SerializeField機能の検証中 (自動)..." -ForegroundColor Yellow

$EngineExe = "$PSScriptRoot/Project/Engine/Bin/Debug/ONEngine.exe"
if (-not (Test-Path $EngineExe)) {
    # フォールバック
    $EngineExe = "$PSScriptRoot/Project/Engine/Bin/Development/ONEngine.exe"
}

if (-not (Test-Path $EngineExe)) {
    Write-Warning "ONEngine.exe が見つかりません。先にエンジンをビルドしてください。"
    exit 1
}

# テストシーンを実行
$TestOutput = "$PSScriptRoot/Generated/verify_serialize_field.json"
if (Test-Path $TestOutput) { Remove-Item $TestOutput }

Write-Host "テスト用シーンを実行しています..."
& $EngineExe --test-mode --test-duration 120 --test-output $TestOutput --test-scene InheritSerializeFieldTest --test-input "$PSScriptRoot/Project/Assets/Tests/InheritSerializeField_test.json" | Out-Null

if (Test-Path $TestOutput) {
    $Result = Get-Content $TestOutput | ConvertFrom-Json
    if ($Result.result -eq "passed") {
        Write-Host "【合格】SerializeField の親子継承デシリアライズは正常に動作しています。" -ForegroundColor Green
    } else {
        Write-Error "【不合格】SerializeField テストシーンの実行結果が異常です。"
    }
    Remove-Item $TestOutput
} else {
    Write-Error "【エラー】テスト結果ファイルが出力されませんでした。"
}

# --- 2. [手動/自動連携] ホットリロードの検証 ---
Write-Host "`n[2/2] ホットリロード機能の検証中 (手動/自動)..." -ForegroundColor Yellow

# テストファイルのパス
$TargetFile = "$PSScriptRoot/SubProjects/CSharpLibrary/Scripts/Test/ProfileTest.cs"
if (-not (Test-Path $TargetFile)) {
    Write-Error "検証ターゲットファイルが見つかりません: $TargetFile"
}

# バックアップ作成
$BackupFile = "${TargetFile}.bak"
Copy-Item $TargetFile $BackupFile

# 一時的な書き換え (Updateに関数を埋め込んで、コンソールにデバッグログを毎フレーム吐くようにする)
$Content = Get-Content $TargetFile -Raw
$HotReloadCode = "`n`t`tDebug.Log(\"=== HOT RELOAD SUCCESS ===\");`n"
$NewContent = $Content -replace 'public override void Update\(\) \{', "public override void Update() {${HotReloadCode}"

Set-Content $TargetFile $NewContent -NoNewline

Write-Host "検証用のコードを一時的に書き込みました。" -ForegroundColor Green
Write-Host "【重要】" -ForegroundColor Cyan
Write-Host "1. ONEngine.exe を通常起動（Debug/Development構成）してください。" -ForegroundColor Cyan
Write-Host "2. 起動を確認したら、この画面で [Enter] キーを押してください。自動でビルドが実行されます。" -ForegroundColor Cyan
Read-Host

Write-Host "ビルド（ホットリロード）を実行しています..."
Push-Location "$PSScriptRoot/SubProjects/CSharpLibrary"
dotnet build CSharpLibrary.csproj --configuration Debug /p:Platform=x64 | Out-Null
Pop-Location

Write-Host "`nビルドが完了しました。最新のDLLが転送されました。" -ForegroundColor Green
Write-Host "【目視確認】" -ForegroundColor Cyan
Write-Host "起動したゲームエンジンのコンソール（ImGuiログなど）に" -ForegroundColor Cyan
Write-Host "『 === HOT RELOAD SUCCESS === 』" -ForegroundColor Green
Write-Host "と連続でログ出力されていることを確認してください。" -ForegroundColor Cyan
Write-Host "確認できたら、この画面で [Enter] キーを押して検証を終了してください。"
Read-Host

# コードの復元
Copy-Item $BackupFile $TargetFile -Force
Remove-Item $BackupFile

Write-Host "検証用コードを元の状態にロールバックしました。" -ForegroundColor Green
Write-Host "`n全ての動作検証が完了しました！" -ForegroundColor Cyan
