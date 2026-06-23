@echo off
setlocal

REM --- バッチファイルと同じフォルダにある PowerShell スクリプトを実行 ---
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0RenameDll.ps1"

REM --- PowerShell の終了コード（exit 0 や 2 など）をそのままバッチの終了コードとして返す ---
exit /b %errorlevel%