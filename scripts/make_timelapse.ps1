# Timelapse generation script for Windows
# This script builds and runs the PNG timelapse example and creates videos

param(
    [int]$Width = 128,
    [int]$Height = 128,
    [int]$Steps = 100,
    [int]$Seed = 42
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Path $MyInvocation.MyCommand.Definition -Parent
$RootDir = Split-Path -Path $ScriptDir -Parent
$Ex = Join-Path $RootDir "examples\pixel_timelapse"

Write-Host "================================================"
Write-Host "Units Timelapse Generation"
Write-Host "Resolution: ${Width}x${Height}, Steps: ${Steps}, Seed: ${Seed}"
Write-Host "Repository root: ${RootDir}"
Write-Host "================================================"

# ===== Build Phase =====
Write-Host ""
Write-Host "Building from repository root with OpenMP and float precision..."

$BuildDir = Join-Path $RootDir "build"
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null
Set-Location $BuildDir

# Try Ninja generator first, fallback to Visual Studio
try {
    cmake -G "Ninja" -DUSE_OPENMP=ON -DUSE_FLOAT=ON -DCMAKE_BUILD_TYPE=Release $RootDir
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
    cmake --build . --target units_pixel_timelapse_core
} catch {
    Write-Host "Ninja failed, trying Visual Studio generator..."
    cmake -G "Visual Studio 17 2022" -DUSE_OPENMP=ON -DUSE_FLOAT=ON -DCMAKE_BUILD_TYPE=Release $RootDir
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
    cmake --build . --config Release --target units_pixel_timelapse_core
}

Set-Location $RootDir

# ===== Core PNG Version =====
Write-Host ""
Write-Host "Running core PNG timelapse example..."

$CoreExe = Join-Path $RootDir "build\examples\pixel_timelapse\units_pixel_timelapse_core.exe"
if (-not (Test-Path $CoreExe)) {
    Write-Host "ERROR: Core executable not found at $CoreExe"
    exit 1
}

& $CoreExe --width $Width --height $Height --steps $Steps --seed $Seed

# Create frames directory if it doesn't exist
$FramesDir = Join-Path $Ex "frames_png_core"
if (-not (Test-Path $FramesDir)) {
    New-Item -ItemType Directory -Path $FramesDir | Out-Null
}

Write-Host "Creating core PNG timelapse video..."
$OutputVideo = Join-Path $Ex "timelapse_colored_core.mp4"

# Check if frames were generated
$FirstFrame = Join-Path $FramesDir "frame_0000.png"
if (-not (Test-Path $FirstFrame)) {
    Write-Host "WARNING: No frames found in $FramesDir"
    exit 1
}

# Use ffmpeg to create video
ffmpeg -y -framerate 25 -i "$FramesDir\frame_%04d.png" -c:v libx264 -pix_fmt yuv420p -preset fast $OutputVideo

if ($LASTEXITCODE -eq 0) {
    Write-Host "Core PNG timelapse written to: $OutputVideo"
} else {
    Write-Host "ERROR: ffmpeg failed with exit code $LASTEXITCODE"
    exit 1
}

Write-Host ""
Write-Host "================================================"
Write-Host "Timelapse generation complete!"
Write-Host "Core PNG video: $OutputVideo"
Write-Host "================================================"
