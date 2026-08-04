param(
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = 'Stop'
$buildRoot = 'C:\Users\formless\Desktop\A_Low_Latency_Limit_Order_Book_LOB_C_RUST_GO_PYTHON\build-coverage'
$existingBuildRoot = Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) 'build'
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Source)
if (-not $cmake) {
    $cmake = 'C:\Program Files\CMake\bin\cmake.exe'
}

if (-not (Test-Path $cmake)) {
    throw "CMake was not found. Install CMake and ensure it is on PATH."
}

function Get-NinjaExecutable {
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Source
    if ($ninja) { return $ninja }

    $commonPaths = @(
        'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake',
        'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake',
        'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake'
    )

    foreach ($path in $commonPaths) {
        foreach ($subPath in @('ninja.exe', 'Ninja\ninja.exe')) {
            $candidate = Join-Path $path $subPath
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    return $null
}

function Get-NMakeExecutable {
    $nmake = Get-Command nmake -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Source
    if ($nmake) { return $nmake }

    $commonPaths = @(
        'C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC',
        'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC',
        'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC'
    )

    foreach ($root in $commonPaths) {
        if (-not (Test-Path $root)) { continue }
        Get-ChildItem -Path $root -Directory -ErrorAction SilentlyContinue | ForEach-Object {
            $candidate = Join-Path $_.FullName 'bin\Hostx64\x64\nmake.exe'
            if (Test-Path $candidate) { return $candidate }
            $candidate = Join-Path $_.FullName 'bin\Hostx64\x86\nmake.exe'
            if (Test-Path $candidate) { return $candidate }
        }
    }

    return $null
}

$effectiveBuildRoot = $buildRoot
if (Test-Path $buildRoot) {
    Remove-Item $buildRoot -Recurse -Force
}

if (-not (Test-Path (Join-Path $buildRoot 'CMakeCache.txt'))) {
    New-Item -ItemType Directory -Path $buildRoot -Force | Out-Null
    $generatorArgs = @('-S', Split-Path -Parent (Split-Path -Parent $PSScriptRoot), '-B', $buildRoot, '-DCMAKE_BUILD_TYPE=Debug', '-DENABLE_COVERAGE=ON')

    if ($IsWindows) {
        if (Get-Command clang-cl -ErrorAction SilentlyContinue) {
            $ninjaExe = Get-NinjaExecutable
            if (-not $ninjaExe) {
                throw "clang-cl was found but Ninja could not be located. Install Ninja or add it to PATH."
            }
            $generatorArgs += @('-G', 'Ninja', "-DCMAKE_MAKE_PROGRAM=$ninjaExe", '-DCMAKE_C_COMPILER=clang-cl', '-DCMAKE_CXX_COMPILER=clang-cl')
        }
        else {
            $nmakeExe = Get-NMakeExecutable
            if ($nmakeExe) {
                $generatorArgs += @('-G', 'NMake Makefiles', "-DCMAKE_MAKE_PROGRAM=$nmakeExe", '-DCMAKE_C_COMPILER=clang-cl', '-DCMAKE_CXX_COMPILER=clang-cl')
            }
            else {
                throw "No supported Windows coverage build generator found. Install clang-cl and Ninja, or install NMake and configure clang-cl."
            }
        }
    }
    else {
        if (Get-Command ninja -ErrorAction SilentlyContinue) {
            $generatorArgs += @('-G', 'Ninja')
        }
        else {
            $generatorArgs += @('-G', 'Unix Makefiles')
        }
    }

    & $cmake @generatorArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configure did not complete for coverage build."
        exit $LASTEXITCODE
    }
}

if (Test-Path (Join-Path $effectiveBuildRoot 'CMakeCache.txt')) {
    & $cmake --build $effectiveBuildRoot --config $Configuration
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$ctest = (Get-Command ctest -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Source)
if (-not $ctest) {
    $ctest = 'C:\Program Files\CMake\bin\ctest.exe'
}
if ($IsWindows) {
    $sharedLibDirs = @(
        "$effectiveBuildRoot\core\$Configuration",
        "$effectiveBuildRoot\tests\$Configuration",
        "$effectiveBuildRoot\apps\$Configuration"
    ) | Where-Object { Test-Path $_ }
    if ($sharedLibDirs.Count -gt 0) {
        $existingPath = $env:PATH -split ';' | Where-Object { $_ -and ($_ -ne '') }
        $env:PATH = ($sharedLibDirs + $existingPath) -join ';'
        Write-Host "Updated PATH for native tests with: $($sharedLibDirs -join ';')"
    }
}
$testDir = Join-Path $effectiveBuildRoot 'tests'
if ((Test-Path $ctest) -and (Test-Path $testDir)) {
    & $ctest --test-dir $testDir -C $Configuration --output-on-failure
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
elseif (-not (Test-Path $ctest)) {
    Write-Host 'ctest was not found; skipping native test execution.'
}
else {
    Write-Host "CTest registration not found at $testDir; skipping native test execution."
}

Write-Host "Coverage build completed. Profiling data can be found under $effectiveBuildRoot"
