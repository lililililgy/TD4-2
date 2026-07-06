param (
    [string]$SceneName = "",
    [string]$InputPath = "",
    [int]$Duration = 120
)

$ErrorActionPreference = "Stop"

# Prevent Git from prompting for credentials in non-interactive environment
$env:GIT_TERMINAL_PROMPT = "0"
$env:GIT_ASKPASS = "echo"

# 1. Verify current branch
$currentBranch = (git branch --show-current).Trim()
Write-Host "Current branch: $currentBranch" -ForegroundColor Cyan

if ($currentBranch -eq "engine" -or $currentBranch -eq "main") {
    Write-Error "Error: Currently on base branch ($currentBranch). Please run on a task branch."
    exit 1
}

# 2. Auto-commit changes
$status = git status --porcelain
if ($status) {
    Write-Host "Uncommitted changes detected. Auto-committing before testing..." -ForegroundColor Yellow
    git add -A
    git commit -m "Auto-commit: Complete task before automated merge"
}

# 3. Run automated tests
Write-Host "Running automated tests..." -ForegroundColor Cyan
try {
    $params = @{}
    if ($SceneName) { $params["SceneName"] = $SceneName }
    if ($InputPath) { $params["InputPath"] = $InputPath }
    $params["Duration"] = $Duration

    & "$PSScriptRoot/RunAutoTest.ps1" @params
} catch {
    Write-Error "Test execution failed. Aborting merge. Error: $_"
    exit 1
}

if ($LASTEXITCODE -ne 0) {
    Write-Error "Tests failed. Aborting merge."
    exit 1
}

Write-Host "Tests passed! Starting automated merge flow." -ForegroundColor Green

# 4. Switch to engine branch and update
Write-Host "Switching to engine branch..." -ForegroundColor Cyan
git checkout engine

Write-Host "Pulling latest updates for engine..." -ForegroundColor Cyan
try {
    git pull origin engine
} catch {
    Write-Warning "Failed to pull from remote origin. Proceeding with merge anyway."
}

# 5. Merge branch
Write-Host "Merging $currentBranch into engine branch..." -ForegroundColor Cyan
$mergeResult = git merge $currentBranch 2>&1
$mergeStatus = $LASTEXITCODE

# Handle conflicts
if ($mergeStatus -ne 0) {
    Write-Host "--------------------------------------------------" -ForegroundColor Red
    Write-Host "Conflict detected during merge!" -ForegroundColor Red
    Write-Host "Aborting merge and rolling back to task branch..." -ForegroundColor Red
    Write-Host "--------------------------------------------------" -ForegroundColor Red
    
    # Abort merge
    git merge --abort
    
    # Back to work branch
    git checkout $currentBranch
    
    Write-Host "Returned to task branch '$currentBranch'." -ForegroundColor Yellow
    Write-Host "Please resolve conflicts manually using these steps:" -ForegroundColor Yellow
    Write-Host " 1. Run 'git merge engine' on your task branch"
    Write-Host " 2. Resolve conflicts in your editor and commit the changes"
    Write-Host " 3. Run .\CompleteAndMergeTask.ps1 again"
    
    exit 1
}

# 6. Push to origin
Write-Host "Pushing merged engine branch to origin..." -ForegroundColor Cyan
try {
    git push origin engine
} catch {
    Write-Warning "Failed to push to remote origin. Local merge is complete."
}

# 7. Cleanup task branch
Write-Host "Deleting completed task branch '$currentBranch'..." -ForegroundColor Cyan
git branch -d $currentBranch

Write-Host "All processes completed successfully!" -ForegroundColor Green
