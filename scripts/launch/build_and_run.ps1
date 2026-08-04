$ErrorActionPreference = "Stop"
$ScriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptPath)

# Change to project root
Set-Location $ProjectRoot

Write-Host "Starting Low-Latency Order Book Engine Build and Run..." -ForegroundColor Cyan
Write-Host "Project root: $ProjectRoot" -ForegroundColor Gray

# Check prerequisites
Write-Host "`n=== Checking Prerequisites ===" -ForegroundColor Cyan

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "Error: CMake not found. Install from https://cmake.org/download/" -ForegroundColor Red
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

$compilerFound = $false
if ($env:VSINSTALLDIR -and (Test-Path $env:VSINSTALLDIR)) {
    Write-Host "MSVC found" -ForegroundColor Green
    $compilerFound = $true
}
elseif (Get-Command clang++ -ErrorAction SilentlyContinue) {
    Write-Host "Clang found" -ForegroundColor Green
    $compilerFound = $true
}
elseif (Get-Command g++ -ErrorAction SilentlyContinue) {
    Write-Host "GCC found" -ForegroundColor Green
    $compilerFound = $true
}

if (-not $compilerFound) {
    Write-Host "Warning: No C++ compiler found. Attempting to build anyway..." -ForegroundColor Yellow
}

Write-Host "Prerequisites check complete" -ForegroundColor Green

# Build C++ core
Write-Host "`n=== Building C++ Core ===" -ForegroundColor Cyan
$BuildDir = Join-Path $ProjectRoot "build"

if (Test-Path $BuildDir) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
}

# Try Conan first, fall back to FetchContent
if (Get-Command conan -ErrorAction SilentlyContinue) {
    Write-Host "Installing dependencies with Conan..." -ForegroundColor Yellow
    conan install . --output-folder=$BuildDir --build=missing
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Warning: Conan install failed, using FetchContent instead" -ForegroundColor Yellow
        Write-Host "Configuring with CMake (FetchContent)..." -ForegroundColor Yellow
        cmake -B $BuildDir -S $ProjectRoot -DCMAKE_BUILD_TYPE=Release
    }
    else {
        Write-Host "Configuring with CMake (Conan)..." -ForegroundColor Yellow
        cmake -B $BuildDir -S $ProjectRoot -DCMAKE_TOOLCHAIN_FILE="$BuildDir\conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
    }
}
else {
    Write-Host "Conan not found, using FetchContent..." -ForegroundColor Yellow
    Write-Host "Configuring with CMake (FetchContent)..." -ForegroundColor Yellow
    cmake -B $BuildDir -S $ProjectRoot -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_APPS=ON
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: CMake configuration failed" -ForegroundColor Red
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

Write-Host "Building C++ core..." -ForegroundColor Yellow
cmake --build $BuildDir --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build failed" -ForegroundColor Red
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

Write-Host "C++ core built successfully" -ForegroundColor Green

# Run C++ tests
Write-Host "`n=== Running C++ Tests ===" -ForegroundColor Cyan
Set-Location $BuildDir
ctest -C Release --output-on-failure
$testResult = $LASTEXITCODE
Set-Location $ProjectRoot

if ($testResult -ne 0) {
    Write-Host "Warning: C++ tests failed" -ForegroundColor Yellow
}
else {
    Write-Host "C++ tests passed" -ForegroundColor Green
}

# Start application
Write-Host "`n=== Starting Application ===" -ForegroundColor Cyan
$CliPath = Join-Path $ProjectRoot "build\apps\Release\lob_cli.exe"

if (Test-Path $CliPath) {
    Write-Host "Launching CLI application..." -ForegroundColor Yellow
    Write-Host "`nAvailable commands:" -ForegroundColor Cyan
    Write-Host "  add id price qty side - Add limit order" -ForegroundColor White
    Write-Host "  cancel id - Cancel order" -ForegroundColor White
    Write-Host "  bid - Get best bid" -ForegroundColor White
    Write-Host "  ask - Get best ask" -ForegroundColor White
    Write-Host "  depth - Get bid/ask depth" -ForegroundColor White
    Write-Host "  pool - Show memory pool statistics" -ForegroundColor White
    Write-Host "  snapshot - Print order book snapshot" -ForegroundColor White
    Write-Host "  benchmark n - Run quick benchmark" -ForegroundColor White
    Write-Host "  help - Show help" -ForegroundColor White
    Write-Host "`n" -ForegroundColor White

    & $CliPath
}
else {
    Write-Host "Error: CLI application not found at $CliPath" -ForegroundColor Red
    exit 1
}

Write-Host "`nBuild and run completed successfully!" -ForegroundColor Green
