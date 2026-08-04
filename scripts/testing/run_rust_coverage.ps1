param(
    [string]$ModuleRoot = "bindings/rust",
    [string]$LcovPath = "artifacts/rust-coverage.lcov",
    [string]$Report = "artifacts/rust-coverage.txt"
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$moduleRoot = Join-Path $repo $ModuleRoot
$lcovPath = Join-Path $repo $LcovPath
$reportPath = Join-Path $repo $Report

New-Item -ItemType Directory -Path (Split-Path $lcovPath -Parent) -Force | Out-Null
Set-Location $moduleRoot

cargo test --quiet
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cargo install cargo-llvm-cov --locked
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cargo llvm-cov --workspace --all-features --lcov --output-path "$lcovPath"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cargo llvm-cov --workspace --all-features --summary-only | Set-Content -Path $reportPath
Write-Host "Rust coverage report written to $reportPath"
