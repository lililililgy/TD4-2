param (
    [Parameter(Mandatory=$true)]
    [string]$BranchName
)

$ErrorActionPreference = "Stop"

# feature/ のプレフィックスを必要に応じて補完
if (-not $BranchName.StartsWith("feature/")) {
    $FullBranchName = "feature/$BranchName"
} else {
    $FullBranchName = $BranchName
}

# 1. 未コミットの変更があるかチェック
$status = git status --porcelain
if ($status) {
    Write-Error "エラー: 未コミットの変更があります。コミットするか git stash で退避してから実行してください。"
    exit 1
}

# 2. engineブランチにチェックアウト
Write-Host "engineブランチにチェックアウトしています..." -ForegroundColor Cyan
git checkout engine

# 3. 最新の変更を取得
Write-Host "最新の変更を取得しています..." -ForegroundColor Cyan
try {
    git pull origin engine
} catch {
    Write-Warning "リモートからのpullに失敗しました。オフライン状態か、リモートが存在しない可能性があります。処理を続行します。"
}

# 4. 新規ブランチを作成・チェックアウト
Write-Host "新規ブランチ '$FullBranchName' を作成し、切り替えています..." -ForegroundColor Cyan
git checkout -b $FullBranchName

Write-Host "完了しました。現在のブランチ: $FullBranchName" -ForegroundColor Green
