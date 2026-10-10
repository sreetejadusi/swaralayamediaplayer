$ErrorActionPreference = "Stop"

$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

Write-Host "============================================="
Write-Host "0. Setting up Qt Environment..."
Write-Host "============================================="
$QtBase = "C:\Qt"
if (Test-Path $QtBase) {
    # Find Qt bin path (e.g., C:\Qt\6.11.1\mingw_64\bin)
    $QtBinInfo = Get-ChildItem -Path $QtBase -Directory -Filter "6.*" | 
                 Get-ChildItem -Directory -Filter "mingw_64" | 
                 Join-Path -ChildPath "bin" | Select-Object -First 1
    
    $QtBin = if ($QtBinInfo) { (Get-Item $QtBinInfo).FullName } else { $null }

    # Find MinGW bin path (e.g., C:\Qt\Tools\mingw1310_64\bin)
    $MinGwBinInfo = Get-ChildItem -Path "$QtBase\Tools" -Directory -Filter "mingw*" | 
                    Join-Path -ChildPath "bin" | Select-Object -First 1
    
    $MinGwBin = if ($MinGwBinInfo) { (Get-Item $MinGwBinInfo).FullName } else { $null }

    if ($QtBin -and $MinGwBin) {
        Write-Host "Found Qt: $QtBin"
        Write-Host "Found MinGW: $MinGwBin"
        $env:PATH = "$QtBin;$MinGwBin;$env:PATH"
    } else {
        Write-Warning "Could not find Qt or MinGW paths inside C:\Qt. Build might fail."
    }
}

Write-Host "============================================="
Write-Host "1. Configuring CMake..."
Write-Host "============================================="
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Set-Location $BuildDir

# We need to tell CMake where Qt is located
$QtPrefix = ""
if ($QtBin) {
    $QtPrefix = (Get-Item $QtBin).Parent.FullName
}

# Configure CMake for Release build using MinGW
if ($QtPrefix) {
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QtPrefix" ..
} else {
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
}

if ($LASTEXITCODE -ne 0) {
    Write-Warning "CMake configuration failed."
    Write-Warning "If you are using MinGW/Qt, make sure you run this script from the 'Qt MinGW Command Prompt'"
    Write-Warning "or ensure your Qt and MinGW bin folders are in your system PATH."
    exit $LASTEXITCODE
}

Write-Host "============================================="
Write-Host "2. Building Project..."
Write-Host "============================================="
cmake --build . --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Warning "Build failed."
    exit $LASTEXITCODE
}

# Determine where the executable actually is (differs between MinGW/Ninja and MSVC)
$ExeDir = $BuildDir
$ExePath = Join-Path $ExeDir "SwaralayaMediaPlayer.exe"
if (-not (Test-Path $ExePath)) {
    $ExeDir = Join-Path $BuildDir "Release"
    $ExePath = Join-Path $ExeDir "SwaralayaMediaPlayer.exe"
    if (-not (Test-Path $ExePath)) {
        Write-Error "Could not find SwaralayaMediaPlayer.exe after build."
        exit 1
    }
}

Write-Host "============================================="
Write-Host "3. Running windeployqt..."
Write-Host "============================================="
# windeployqt must be in your PATH (from your Qt installation, e.g., C:\Qt\6.x.x\mingw_64\bin)
try {
    windeployqt --release $ExePath
} catch {
    Write-Warning "windeployqt failed or was not found in PATH."
    Write-Warning "Please ensure your Qt 'bin' folder is in your system PATH."
    exit 1
}

Write-Host "============================================="
Write-Host "4. Compiling Inno Setup Installer..."
Write-Host "============================================="
Set-Location $ProjectRoot
$IssPath = Join-Path $ProjectRoot "setup.iss"

# Look for ISCC.exe in typical installation paths
$IsccPaths = @(
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    "C:\Program Files\Inno Setup 6\ISCC.exe",
    "C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
)

$Iscc = $null
foreach ($Path in $IsccPaths) {
    if (Test-Path $Path) {
        $Iscc = $Path
        break
    }
}

if (-not $Iscc) {
    # Fallback to checking PATH
    $Iscc = Get-Command "ISCC.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Path
}

if (-not $Iscc) {
    Write-Warning "ISCC.exe (Inno Setup Compiler) not found."
    Write-Warning "Please ensure Inno Setup is installed and ISCC.exe is in PATH."
    exit 1
}

# Run Inno Setup Compiler, overriding MyBuildDir to point to the actual folder containing the .exe
& $Iscc "/DMyBuildDir=$ExeDir" $IssPath

Write-Host "============================================="
Write-Host "Done! Your installer is located in the 'Output' directory."
Write-Host "============================================="
