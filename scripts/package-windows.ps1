param(
    [string]$BuildDir = "build",
    [string]$OutputDir = "dist",
    [string]$PackageName = "blackbox-windows-x64",
    [string]$MsysBin = "C:\msys64\mingw64\bin"
)

$ErrorActionPreference = "Stop"

$root = (Resolve-Path ".").Path
$buildPath = Join-Path $root $BuildDir
$outputRoot = Join-Path $root $OutputDir
$packageDir = Join-Path $outputRoot $PackageName
$zipPath = Join-Path $outputRoot "$PackageName.zip"

New-Item -ItemType Directory -Force -Path $outputRoot | Out-Null

$resolvedOutputRoot = (Resolve-Path $outputRoot).Path
$packageParent = Split-Path -Parent $packageDir
if ((Resolve-Path $packageParent).Path -ne $resolvedOutputRoot) {
    throw "Package directory must stay inside the output directory."
}

Remove-Item -LiteralPath $packageDir -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $zipPath -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $packageDir | Out-Null

$guiExe = Join-Path $buildPath "blackbox_gui.exe"
if (!(Test-Path $guiExe)) {
    throw "Missing GUI executable: $guiExe"
}

Copy-Item -LiteralPath $guiExe -Destination (Join-Path $packageDir "BlackBox.exe")

$cliExe = Join-Path $buildPath "blackbox.exe"
if (Test-Path $cliExe) {
    Copy-Item -LiteralPath $cliExe -Destination (Join-Path $packageDir "blackbox-cli.exe")
}

foreach ($doc in @("README.md", "LICENSE", "LICENSE.txt")) {
    $docPath = Join-Path $root $doc
    if (Test-Path $docPath) {
        Copy-Item -LiteralPath $docPath -Destination $packageDir
    }
}

$deployTool = Join-Path $MsysBin "windeployqt6.exe"
if (!(Test-Path $deployTool)) {
    $deployTool = Join-Path $MsysBin "windeployqt-qt6.exe"
}
if (!(Test-Path $deployTool)) {
    throw "Could not find windeployqt6.exe or windeployqt-qt6.exe in $MsysBin"
}

$env:PATH = "$MsysBin;$env:PATH"

& $deployTool --release --no-translations (Join-Path $packageDir "BlackBox.exe")
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed with exit code $LASTEXITCODE"
}

foreach ($dll in @(
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
    "libsqlite3-0.dll"
)) {
    $dllPath = Join-Path $MsysBin $dll
    if (Test-Path $dllPath) {
        Copy-Item -LiteralPath $dllPath -Destination $packageDir -Force
    } else {
        Write-Warning "Missing runtime DLL: $dllPath"
    }
}

Compress-Archive -Path (Join-Path $packageDir "*") -DestinationPath $zipPath -Force
Write-Host "Created package: $zipPath"
