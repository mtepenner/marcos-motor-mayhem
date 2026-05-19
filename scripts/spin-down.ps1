$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$stateFile = Join-Path $repoRoot ".run/matchmaker.pid"
$redisContainer = "mmm-redis"

if (Test-Path $stateFile) {
    $pid = Get-Content $stateFile -ErrorAction SilentlyContinue
    if ($pid) {
        Write-Host "[spin-down] Stopping matchmaker PID $pid..."
        Stop-Process -Id $pid -Force -ErrorAction SilentlyContinue
    }
    Remove-Item $stateFile -ErrorAction SilentlyContinue
} else {
    Write-Host "[spin-down] No saved matchmaker PID found."
}

if (Get-Command docker -ErrorAction SilentlyContinue) {
    $runningContainer = docker ps --filter "name=^$redisContainer$" --format "{{.Names}}"
    if ($runningContainer) {
        Write-Host "[spin-down] Stopping Redis container '$redisContainer'..."
        docker stop $redisContainer | Out-Null
    }
}

Write-Host "[spin-down] Done."
