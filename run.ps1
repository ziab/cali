<# .SYNOPSIS
  Build and run Cali (DirectX 11).
.DESCRIPTION
  Wrapper around CMake presets.
  - dev / debug   -> Debug build (build/bin/Debug/cali.exe)
  - release / rel -> Release build (build/bin/Release/cali.exe)
  Handles configure, build and launch. Multi-config VS generator is used.
  Tests are NOT run by default (use -Test to run them).
.EXAMPLE
  .\run.ps1               # dev build + run (default)
  .\run.ps1 dev           # same
  .\run.ps1 release       # release build + run
  .\run.ps1 dev -Clean    # clean configure + build + run
  .\run.ps1 release -NoBuild  # just run existing release exe
  .\run.ps1 release -Test     # build + run tests + run app
  .\run.ps1 dev -Test -NoRun  # build + run tests only
  .\run.ps1 -Help
#>
[CmdletBinding()]
param(
    [Parameter(Position=0)]
    [ValidateSet("dev","debug","Debug","release","Release","rel")]
    [string]$Configuration = "dev",

    [switch]$Clean,
    [switch]$NoBuild,
    [switch]$NoRun,
    [switch]$Test,
    [switch]$Help
)

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Detailed
    exit 0
}

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Normalize config ------------------------------------------------------------
$cfgLower = $Configuration.ToLowerInvariant()
if ($cfgLower -in @("dev","debug")) {
    $cmakeConfig = "Debug"
    $preset = "windows-x64-debug"
} else {
    $cmakeConfig = "Release"
    $preset = "windows-x64-release"
}

$root = $PSScriptRoot
if (-not $root) { $root = (Get-Location).Path }
$buildDir = Join-Path $root "build"
# CMake now puts exes in build/bin/<Config> (libs in build/lib/<Config>)
$exe = Join-Path $buildDir "bin\$cmakeConfig\cali.exe"
$cmakeCache = Join-Path $buildDir "CMakeCache.txt"

Write-Host "==> Cali run.ps1  [$cmakeConfig]  preset=$preset" -ForegroundColor Cyan

# Clean -----------------------------------------------------------------------
if ($Clean) {
    if (Test-Path $buildDir) {
        Write-Host "Cleaning $buildDir ..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force $buildDir
    }
    # Also clean legacy VS artefacts if requested
    if (Test-Path (Join-Path $root "x64")) {
        Write-Host "Cleaning x64/ ..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force (Join-Path $root "x64") -ErrorAction SilentlyContinue
    }
}

# Configure -------------------------------------------------------------------
if (-not $NoBuild) {
    if (-not (Test-Path $cmakeCache)) {
        Write-Host "Configuring (cmake --preset $preset) ..." -ForegroundColor Cyan
        $cmakeArgs = @("--preset", $preset)
        # Fallback if cmake < 3.21 doesn't support --preset
        & cmake @cmakeArgs
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Preset failed, trying manual configure..." -ForegroundColor Yellow
            New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
            & cmake -S $root -B $buildDir -G "Visual Studio 17 2022" -A x64
            if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }
        }
    } else {
        Write-Host "Build dir already configured, skipping configure." -ForegroundColor DarkGray
    }

    Write-Host "Building (cmake --build --config $cmakeConfig) ..." -ForegroundColor Cyan
    & cmake --build $buildDir --config $cmakeConfig --parallel
    if ($LASTEXITCODE -ne 0) { throw "build failed" }

    if ($Test) {
        Write-Host "Running tests (ctest -C $cmakeConfig) ..." -ForegroundColor Cyan
        & ctest --test-dir $buildDir -C $cmakeConfig --output-on-failure
        if ($LASTEXITCODE -ne 0) { throw "tests failed" }
    }
}

# Run -------------------------------------------------------------------------
if (-not $NoRun) {
    if (-not (Test-Path $exe)) {
        # Fallbacks for old layout (build/<Config>/cali.exe, build/bin/cali.exe, Ninja)
        $candidates = @(
            (Join-Path $buildDir "$cmakeConfig\cali.exe"),
            (Join-Path $buildDir "bin\cali.exe"),
            (Join-Path $buildDir "cali.exe")
        )
        foreach ($c in $candidates) { if (Test-Path $c) { $exe = $c; break } }
        if (-not (Test-Path $exe) -and (Test-Path (Join-Path $root "src\cali\caliD3D11_d.exe"))) {
            $exe = Join-Path $root "src\cali\caliD3D11_d.exe"
            Write-Host "Using legacy exe $exe" -ForegroundColor Yellow
        } else {
            throw "Executable not found at $exe. Build first or use -NoRun to skip."
        }
    }
    Write-Host "Launching $exe ..." -ForegroundColor Green
    # Ensure shaders are next to exe (CMake post-build already copies, but legacy needs manual)
    $shaderSrc = Join-Path $root "src\cali\shaders"
    $shaderDst = Join-Path (Split-Path $exe) "shaders"
    if ((Test-Path $shaderSrc) -and -not (Test-Path $shaderDst)) {
        Write-Host "Copying shaders to $(Split-Path $exe) ..." -ForegroundColor Yellow
        Copy-Item -Recurse -Force $shaderSrc $shaderDst
    }
    $fontSrc = Join-Path $root "src\cali\courier_new.spritefont"
    if (Test-Path $fontSrc) {
        Copy-Item -Force $fontSrc (Split-Path $exe) -ErrorAction SilentlyContinue
    }

    Push-Location (Split-Path $exe)
    try {
        & $exe
        $code = $LASTEXITCODE
        Write-Host "Cali exited with code $code" -ForegroundColor DarkGray
        exit $code
    } finally {
        Pop-Location
    }
}
