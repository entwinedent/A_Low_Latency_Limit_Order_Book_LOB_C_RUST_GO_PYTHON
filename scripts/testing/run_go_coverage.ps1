param(
    [string]$ModuleRoot = "bindings/go",
    [string]$Output = "artifacts/go-coverage.out",
    [string]$Report = "artifacts/go-coverage.txt"
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$moduleRoot = Join-Path $repo $ModuleRoot
$outputPath = Join-Path $repo $Output
$reportPath = Join-Path $repo $Report

New-Item -ItemType Directory -Path (Split-Path $outputPath -Parent) -Force | Out-Null
Set-Location $moduleRoot

go test ./... -coverprofile="$outputPath" -covermode=count
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

go tool cover -func "$outputPath" | Set-Content -Path $reportPath
Write-Host "Go coverage report written to $reportPath"
