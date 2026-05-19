param(
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Debug",
    [int]$MatchmakerPort = 50051,
    [string]$RedisAddress = "localhost:6379"
)
$ErrorActionPreference = "Stop"
if ($null -ne (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue)) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

function Add-PathIfExists {
    param(
        [Parameter(Mandatory = $true)]
        [string]$PathToAdd
    )

    if ((Test-Path $PathToAdd) -and ($env:PATH -notlike "*$PathToAdd*")) {
        $env:PATH = "$PathToAdd;$env:PATH"
    }
}

# Ensure required tools are reachable when launched from different shells/environments.
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Add-PathIfExists -PathToAdd "C:\Program Files\CMake\bin"
}
if (-not (Get-Command go -ErrorAction SilentlyContinue)) {
    Add-PathIfExists -PathToAdd "C:\Program Files\Go\bin"
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake not found. Install with: winget install --id Kitware.CMake -e"
}
if (-not (Get-Command go -ErrorAction SilentlyContinue)) {
    throw "Go not found. Install with: winget install --id GoLang.Go -e"
}

if (-not $env:VULKAN_SDK) {
    $vulkanRoot = "C:\VulkanSDK"
    if (Test-Path $vulkanRoot) {
        $latestVulkan = Get-ChildItem -Path $vulkanRoot -Directory | Sort-Object Name -Descending | Select-Object -First 1
        if ($latestVulkan) {
            $env:VULKAN_SDK = $latestVulkan.FullName
            Add-PathIfExists -PathToAdd (Join-Path $latestVulkan.FullName "Bin")
        }
    }
}

$stateDir = Join-Path $repoRoot ".run"
$stateFile = Join-Path $stateDir "matchmaker.pid"
if (-not (Test-Path $stateDir)) {
    New-Item -ItemType Directory -Path $stateDir | Out-Null
}

$redisContainer = "mmm-redis"
$startedRedis = $false
$dockerPipe = "\\.\pipe\dockerDesktopLinuxEngine"
function Invoke-DockerSafely {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    $output = & docker @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        return $null
    }

    return $output
}

if ((Get-Command docker -ErrorAction SilentlyContinue) -and (Test-Path $dockerPipe)) {
    $existingContainer = Invoke-DockerSafely -Arguments @("ps", "-a", "--filter", "name=^$redisContainer$", "--format", "{{.Names}}")
    if ($null -eq $existingContainer) {
        Write-Host "[spin-up] Docker is installed but the daemon is unreachable. Assuming Redis is available at $RedisAddress"
    } else {
        if (-not $existingContainer) {
            Write-Host "[spin-up] Starting Redis container '$redisContainer'..."
            $started = Invoke-DockerSafely -Arguments @("run", "-d", "--name", $redisContainer, "-p", "6379:6379", "redis:7-alpine")
            if ($null -ne $started) {
                $startedRedis = $true
            }
        } else {
            $runningContainer = Invoke-DockerSafely -Arguments @("ps", "--filter", "name=^$redisContainer$", "--format", "{{.Names}}")
            if (-not $runningContainer) {
                Write-Host "[spin-up] Starting existing Redis container '$redisContainer'..."
                $started = Invoke-DockerSafely -Arguments @("start", $redisContainer)
                if ($null -ne $started) {
                    $startedRedis = $true
                }
            }
        }
    }
} else {
    Write-Host "[spin-up] Docker daemon not available. Assuming Redis is already available at $RedisAddress"
}

Write-Host "[spin-up] Configuring CMake ($BuildType)..."
cmake -B build -DCMAKE_BUILD_TYPE=$BuildType

Write-Host "[spin-up] Building game ($BuildType)..."
cmake --build build --config $BuildType

$matchmakerOutput = Join-Path $repoRoot "build/matchmaker.exe"
$matchmakerSource = Join-Path $repoRoot "src/cmd/matchmaker"

Write-Host "[spin-up] Building matchmaker..."
Push-Location $matchmakerSource
try {
    go build -o $matchmakerOutput main.go
} finally {
    Pop-Location
}

if (-not (Test-Path $matchmakerOutput)) {
    throw "Matchmaker executable not found at $matchmakerOutput"
}

if (Test-Path $stateFile) {
    $existingPid = Get-Content $stateFile -ErrorAction SilentlyContinue
    if ($existingPid) {
        Stop-Process -Id $existingPid -Force -ErrorAction SilentlyContinue
    }
    Remove-Item $stateFile -ErrorAction SilentlyContinue
}

Write-Host "[spin-up] Starting matchmaker on port $MatchmakerPort..."
$matchmakerProcess = Start-Process -FilePath $matchmakerOutput -ArgumentList @("--port", "$MatchmakerPort", "--redis", $RedisAddress) -PassThru -WindowStyle Hidden
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
