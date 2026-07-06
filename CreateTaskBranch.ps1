param (
    [Parameter(Mandatory=$true)]
    [string]$BranchName
)

$ErrorActionPreference = "Stop"

if (-not $BranchName.StartsWith("feature/")) {
    $FullBranchName = "feature/$BranchName"
} else {
    $FullBranchName = $BranchName
}

# 1. Check for uncommitted changes
$status = git status --porcelain
if ($status) {
    Write-Error "Error: Uncommitted changes detected. Please commit or stash them before creating a new branch."
    exit 1
}

# 2. Checkout engine
Write-Host "Checking out engine branch..." -ForegroundColor Cyan
git checkout engine

# 3. Pull latest changes
Write-Host "Pulling latest changes for engine..." -ForegroundColor Cyan
try {
    git pull origin engine
} catch {
    Write-Warning "Failed to pull from remote origin. Proceeding locally."
}

# 4. Create and checkout new branch
Write-Host "Creating and switching to new branch '$FullBranchName'..." -ForegroundColor Cyan
git checkout -b $FullBranchName

Write-Host "Success! Current branch: $FullBranchName" -ForegroundColor Green
