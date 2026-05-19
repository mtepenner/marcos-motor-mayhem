param(
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Debug",
    [int]$MatchmakerPort = 50051,
    [string]$RedisAddress = "localhost:6379"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$stateDir = Join-Path $repoRoot ".run"
$stateFile = Join-Path $stateDir "matchmaker.pid"
if (-not (Test-Path $stateDir)) {
    New-Item -ItemType Directory -Path $stateDir | Out-Null
}

$redisContainer = "mmm-redis"
$startedRedis = $false

if (Get-Command docker -ErrorAction SilentlyContinue) {
    $existingContainer = docker ps -a --filter "name=^$redisContainer$" --format "{{.Names}}"
    if (-not $existingContainer) {
        Write-Host "[spin-up] Starting Redis container '$redisContainer'..."
        docker run -d --name $redisContainer -p 6379:6379 redis:7-alpine | Out-Null
        $startedRedis = $true
    } else {
        $runningContainer = docker ps --filter "name=^$redisContainer$" --format "{{.Names}}"
        if (-not $runningContainer) {
            Write-Host "[spin-up] Starting existing Redis container '$redisContainer'..."
            docker start $redisContainer | Out-Null
            $startedRedis = $true
        }
    }
} else {
    Write-Host "[spin-up] Docker not found. Assuming Redis is already available at $RedisAddress"
}

Write-Host "[spin-up] Configuring CMake ($BuildType)..."
cmake -B build -DCMAKE_BUILD_TYPE=$BuildType

Write-Host "[spin-up] Building game ($BuildType)..."
cmake --build build --config $BuildType

$matchmakerOutput = Join-Path $repoRoot "build/matchmaker"
$matchmakerSource = Join-Path $repoRoot "src/cmd/matchmaker"

Write-Host "[spin-up] Building matchmaker..."
Push-Location $matchmakerSource
try {
    go build -o $matchmakerOutput main.go
} finally {
    Pop-Location
}

if (Test-Path $stateFile) {
    $existingPid = Get-Content $stateFile -ErrorAction SilentlyContinue
    if ($existingPid) {
        Stop-Process -Id $existingPid -Force -ErrorAction SilentlyContinue
    }
    Remove-Item $stateFile -ErrorAction SilentlyContinue
}

Write-Host "[spin-up] Starting matchmaker on port $MatchmakerPort..."
$matchmakerProcess = Start-Process -FilePath $matchmakerOutput -ArgumentList @("--port", "$MatchmakerPort", "--redis", $RedisAddress) -PassThru -WindowStyle Normal
Set-Content -Path $stateFile -Value $matchmakerProcess.Id

$gameCandidates = @(
    (Join-Path $repoRoot "build/bin/$BuildType/marcos-motor-mayhem.exe"),
    (Join-Path $repoRoot "build/bin/marcos-motor-mayhem.exe")
)

$gamePath = $gameCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $gamePath) {
    throw "Game executable not found. Looked in: $($gameCandidates -join ', ')"
}

Write-Host "[spin-up] Launching game: $gamePath"
Write-Host "[spin-up] Matchmaker PID: $($matchmakerProcess.Id)"
if ($startedRedis) {
    Write-Host "[spin-up] Redis container '$redisContainer' is running"
}

Start-Process -FilePath $gamePath | Out-Null
Write-Host "[spin-up] Stack started. Use scripts/spin-down.ps1 to stop services."
