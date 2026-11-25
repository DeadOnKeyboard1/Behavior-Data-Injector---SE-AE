$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $Root

$Log = Join-Path $Root "build-universal.log"
Start-Transcript -Path $Log -Force | Out-Null

function Require-Command([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Required command '$Name' was not found in PATH."
    }
}

function Ensure-GitCheckout {
    param(
        [Parameter(Mandatory=$true)][string]$Path,
        [Parameter(Mandatory=$true)][string]$Url,
        [Parameter(Mandatory=$true)][string]$Ref
    )

    if (-not (Test-Path (Join-Path $Path ".git"))) {
        if (Test-Path $Path) {
            Remove-Item -Recurse -Force $Path
        }
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $Path) | Out-Null
        & git clone --filter=blob:none --no-checkout $Url $Path
        if ($LASTEXITCODE -ne 0) { throw "git clone failed for $Url" }
    }

    if ($Ref -match '^v[0-9]') {
        & git -C $Path fetch --force --depth 1 origin "refs/tags/${Ref}:refs/tags/${Ref}"
        if ($LASTEXITCODE -ne 0) { throw "git fetch failed for $Url tag $Ref" }

        & git -C $Path checkout --detach "refs/tags/$Ref"
        if ($LASTEXITCODE -ne 0) { throw "git checkout failed for $Url tag $Ref" }
    } else {
        & git -C $Path fetch --force --depth 1 origin $Ref
        if ($LASTEXITCODE -ne 0) { throw "git fetch failed for $Url ref $Ref" }

        & git -C $Path checkout --detach FETCH_HEAD
        if ($LASTEXITCODE -ne 0) { throw "git checkout failed for $Url ref $Ref" }
    }
}

try {
Require-Command git
Require-Command cmake

$Extern = Join-Path $Root "extern"
New-Item -ItemType Directory -Force -Path $Extern | Out-Null

$CommonLib = Join-Path $Extern "CommonLibSSE-NG"
$DKUtil = Join-Path $Extern "DKUtil"

Write-Host "[1/5] Preparing CommonLibSSE-NG v12.1.0..."
Ensure-GitCheckout -Path $CommonLib -Url "https://github.com/alandtse/CommonLibSSE-NG.git" -Ref "v12.1.0"
& git -C $CommonLib submodule update --init --recursive
if ($LASTEXITCODE -ne 0) { throw "CommonLibSSE-NG submodule initialization failed." }

Write-Host "[2/5] Preparing DKUtil..."
Ensure-GitCheckout -Path $DKUtil -Url "https://github.com/gottyduke/DKUtil.git" -Ref "0ddab238cc8c4915b9e8fda90668d74e513d534a"

Write-Host "[3/5] Preparing vcpkg..."
if ([string]::IsNullOrWhiteSpace($env:VCPKG_ROOT)) {
    $Vcpkg = Join-Path $Extern "vcpkg"
    if (-not (Test-Path (Join-Path $Vcpkg ".git"))) {
        & git clone https://github.com/microsoft/vcpkg.git $Vcpkg
        if ($LASTEXITCODE -ne 0) { throw "Could not clone vcpkg." }
    }
    $env:VCPKG_ROOT = $Vcpkg
} else {
    $Vcpkg = $env:VCPKG_ROOT
}

$VcpkgExe = Join-Path $Vcpkg "vcpkg.exe"
if (-not (Test-Path $VcpkgExe)) {
    $Bootstrap = Join-Path $Vcpkg "bootstrap-vcpkg.bat"
    if (-not (Test-Path $Bootstrap)) {
        throw "bootstrap-vcpkg.bat not found in VCPKG_ROOT: $Vcpkg"
    }
    & $Bootstrap -disableMetrics
    if ($LASTEXITCODE -ne 0) { throw "vcpkg bootstrap failed." }
}

Write-Host "[4/5] Configuring and building BehaviorDataInjector..."
$Build = Join-Path $Root "build"
if (Test-Path $Build) {
    Remove-Item -Recurse -Force $Build
}

$Toolchain = Join-Path $Vcpkg "scripts\buildsystems\vcpkg.cmake"

& cmake `
    -S $Root `
    -B $Build `
    -G "Visual Studio 17 2022" `
    -A x64 `
    "-DCMAKE_TOOLCHAIN_FILE=$Toolchain" `
    "-DVCPKG_TARGET_TRIPLET=x64-windows-static-md" `
    "-DMOD_AUTOMATED_BUILD=ON"

if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

& cmake --build $Build --config Release --target BehaviorDataInjector -- /m
if ($LASTEXITCODE -ne 0) { throw "CMake build failed." }

Write-Host "[5/5] Packaging DLL + PDB..."
$Dll = Get-ChildItem -Path $Build -Recurse -Filter "BehaviorDataInjector.dll" |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1
$Pdb = Get-ChildItem -Path $Build -Recurse -Filter "BehaviorDataInjector.pdb" |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1

if (-not $Dll) {
    throw "Build completed but BehaviorDataInjector.dll was not found."
}
if (-not $Pdb) {
    throw "Build completed but BehaviorDataInjector.pdb was not found."
}

$Dist = Join-Path $Root "dist"
$PluginDir = Join-Path $Dist "SKSE\Plugins"
if (Test-Path $Dist) {
    Remove-Item -Recurse -Force $Dist
}
New-Item -ItemType Directory -Force -Path $PluginDir | Out-Null

Copy-Item $Dll.FullName (Join-Path $PluginDir "BehaviorDataInjector.dll") -Force
# PDB is kept in $Build for developer crash analysis and not packaged to prevent leaking MSVC CRT toolchain paths.

$IniContent = @"
[Debug]
; Enable debug logging to Documents\My Games\Skyrim Special Edition\SKSE\BehaviorDataInjector.log
EnableDebugLog = false
"@
Set-Content -Path (Join-Path $PluginDir "BehaviorDataInjector.ini") -Value $IniContent -Encoding utf8

Copy-Item (Join-Path $Root "LICENSE") (Join-Path $Dist "LICENSE") -Force
Copy-Item (Join-Path $Root "EXCEPTIONS.md") (Join-Path $Dist "EXCEPTIONS.md") -Force
Copy-Item (Join-Path $Root "NOTICE") (Join-Path $Dist "NOTICE") -Force

$Zip = Join-Path $Root "BehaviorDataInjector-Universal-1.7.104-Verified-Vortex.zip"
if (Test-Path $Zip) {
    Remove-Item -Force $Zip
}

Compress-Archive -Path (Join-Path $Dist "*") -DestinationPath $Zip -CompressionLevel Optimal

Write-Host ""
Write-Host "SUCCESS"
Write-Host "DLL: $($Dll.FullName)"
Write-Host "PDB: $($Pdb.FullName)"
Write-Host "Package: $Zip"
}
finally {
    try {
        Stop-Transcript | Out-Null
    } catch {
        # Do not mask the original build error.
    }
    if (Test-Path $Log) {
        $content = Get-Content -LiteralPath $Log -Raw
        $user = $env:USERNAME
        $profile = $env:USERPROFILE
        $computer = $env:COMPUTERNAME
        if ($profile) { $content = $content -replace [regex]::Escape($profile), '<USER>' }
        if ($user) { $content = $content -replace [regex]::Escape($user), '<USER>' }
        if ($computer) { $content = $content -replace [regex]::Escape($computer), '<HOST>' }
        [IO.File]::WriteAllText($Log, $content, [Text.Encoding]::UTF8)
    }
}

