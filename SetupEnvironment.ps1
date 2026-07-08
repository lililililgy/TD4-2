# SetupEnvironment.ps1
# ONEngine C# 開発環境自動構築スクリプト

$ErrorActionPreference = "Stop"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "      ONEngine 開発環境セットアップ      " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# 1. パスの全角文字チェック
Write-Host "`n[1/3] パスの確認中..." -ForegroundColor Yellow
if ($PSScriptRoot -match "[^\x00-\x7F]") {
    Write-Warning "【警告】プロジェクトの配置パスに全角文字（日本語など）が含まれています。"
    Write-Warning "この状態ではMonoランタイムが正常に動作せず、ゲームがクラッシュします。"
    Write-Warning "すべて半角英数字のパス（例: C:\Develop\ONEngine）へプロジェクトを移動させてください。"
} else {
    Write-Host "パスの検証に成功しました（すべて半角英数字です）。" -ForegroundColor Green
}

# 2. VS Code 拡張機能のインストール
Write-Host "`n[2/3] VS Code デバッグ拡張機能のインストール中..." -ForegroundColor Yellow
if (Get-Command "code" -ErrorAction SilentlyContinue) {
    try {
        Write-Host "Mono Debug をインストールしています..."
        code --install-extension ms-vscode.mono-debug --force | Out-Null
        
        Write-Host "C# 言語サポートをインストールしています..."
        code --install-extension ms-dotnettools.csharp --force | Out-Null
        
        Write-Host "VS Code 拡張機能のインストールが完了しました。" -ForegroundColor Green
    } catch {
        Write-Warning "VS Code 拡張機能の自動インストール中にエラーが発生しました。手動でインストールしてください。"
    }
} else {
    Write-Warning "code コマンドが見つかりませんでした。VS Codeがインストールされていないか、PATHが通っていない可能性があります。"
    Write-Warning "VS Code にて手動で 'Mono Debug' および 'C#' 拡張機能をインストールしてください。"
}

# 3. プロジェクトファイルの再生成 (Premake)
Write-Host "`n[3/3] プロジェクトファイルの生成中..." -ForegroundColor Yellow
try {
    # C++
    Write-Host "C++エンジンプロジェクトの生成中..."
    Push-Location "$PSScriptRoot/Project"
    powershell -NoProfile -ExecutionPolicy Bypass -File .\GenerateProject.ps1 | Out-Null
    Pop-Location

    # C#
    Write-Host "C#ライブラリプロジェクトの生成中..."
    Push-Location "$PSScriptRoot/SubProjects/CSharpLibrary"
    powershell -NoProfile -ExecutionPolicy Bypass -File .\GenerateProject_CS.ps1 | Out-Null
    Pop-Location

    Write-Host "プロジェクトファイルの生成が完了しました。" -ForegroundColor Green
} catch {
    Write-Error "プロジェクトファイルの生成に失敗しました: $_"
}

Write-Host "`nセットアップがすべて完了しました！" -ForegroundColor Cyan
Write-Host "エンジンを Debug / Development 構成で起動し、VS Code からアタッチしてデバッグを開始できます。" -ForegroundColor Cyan
