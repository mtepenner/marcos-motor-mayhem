$ErrorActionPreference = "Stop"
if ($null -ne (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue)) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$stateFile = Join-Path $repoRoot ".run/matchmaker.pid"
$redisContainer = "mmm-redis"
$dockerPipe = "\\.\pipe\dockerDesktopLinuxEngine"

if (Test-Path $stateFile) {
    $matchmakerPid = Get-Content $stateFile -ErrorAction SilentlyContinue
    if ($matchmakerPid) {
        Write-Host "[spin-down] Stopping matchmaker PID $matchmakerPid..."
        Stop-Process -Id $matchmakerPid -Force -ErrorAction SilentlyContinue
    }
    Remove-Item $stateFile -ErrorAction SilentlyContinue
} else {
    Write-Host "[spin-down] No saved matchmaker PID found."
}

if ((Get-Command docker -ErrorAction SilentlyContinue) -and (Test-Path $dockerPipe)) {
    $runningContainer = docker ps --filter "name=^$redisContainer$" --format "{{.Names}}"
    if ($runningContainer) {
        Write-Host "[spin-down] Stopping Redis container '$redisContainer'..."
        docker stop $redisContainer | Out-Null
    }
}

Write-Host "[spin-down] Done."
