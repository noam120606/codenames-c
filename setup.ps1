$ErrorActionPreference = 'Stop'

# =============================
# Configuration
# =============================
$SDL_VERSION = '2.30.0'
$SDL_IMAGE_VERSION = '2.8.2'
$SDL_TTF_VERSION = '2.22.0'
$SDL_MIXER_VERSION = '2.8.0'

$ROOT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$DEPS_DIR = Join-Path $ROOT_DIR 'client'
$BUILD_DIR = Join-Path $DEPS_DIR 'build_SDL2_win'
$INSTALL_DIR = Join-Path $DEPS_DIR 'SDL2'

# Choix toolchain: detecte l'architecture du gcc present
$MINGW_TRIPLET = 'x86_64-w64-mingw32'
if (Get-Command gcc -ErrorAction SilentlyContinue) {
    try {
        $machine = (& gcc -dumpmachine).Trim()
        if ($machine -match 'x86_64') {
            $MINGW_TRIPLET = 'x86_64-w64-mingw32'
        } elseif ($machine -match 'i686|mingw32') {
            $MINGW_TRIPLET = 'i686-w64-mingw32'
        }
    } catch {
        # garder la valeur par defaut
    }
}

# =============================
# Checks minimaux
# =============================
if (-not (Get-Command tar -ErrorAction SilentlyContinue)) {
    throw 'tar est requis (bsdtar integre a Windows 10/11).'
}

# =============================
# Preparation des dossiers
# =============================
New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null
New-Item -ItemType Directory -Force -Path $INSTALL_DIR | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $INSTALL_DIR 'include') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $INSTALL_DIR 'include/SDL2') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $INSTALL_DIR 'lib') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $INSTALL_DIR 'bin') | Out-Null

Set-Location $BUILD_DIR

function Download-IfMissing {
    param(
        [Parameter(Mandatory = $true)][string]$FileName,
        [Parameter(Mandatory = $true)][string]$Url
    )

    if (-not (Test-Path $FileName)) {
        Write-Host "Telechargement de $FileName..."
        Invoke-WebRequest -Uri $Url -OutFile $FileName
    }
}

function Install-SdlPackage {
    param(
        [Parameter(Mandatory = $true)][string]$ArchiveName,
        [Parameter(Mandatory = $true)][string]$ExtractedDirName,
        [Parameter(Mandatory = $true)][string]$PrettyName
    )

    Write-Host "Installation $PrettyName..."
    tar -xzf $ArchiveName

    $packageRoot = Join-Path $BUILD_DIR $ExtractedDirName
    $mingwRoot = Join-Path $packageRoot $MINGW_TRIPLET

    if (-not (Test-Path $mingwRoot)) {
        $fallbackTriplet = if ($MINGW_TRIPLET -eq 'x86_64-w64-mingw32') { 'i686-w64-mingw32' } else { 'x86_64-w64-mingw32' }
        $fallbackRoot = Join-Path $packageRoot $fallbackTriplet
        if (Test-Path $fallbackRoot) {
            $mingwRoot = $fallbackRoot
        } else {
            throw "Impossible de trouver $mingwRoot"
        }
    }

    $srcInclude = Join-Path $mingwRoot 'include/SDL2'
    if (Test-Path $srcInclude) {
        Copy-Item -Path (Join-Path $srcInclude '*') -Destination (Join-Path $INSTALL_DIR 'include/SDL2') -Recurse -Force
    }

    $srcLib = Join-Path $mingwRoot 'lib'
    if (Test-Path $srcLib) {
        Copy-Item -Path (Join-Path $srcLib '*') -Destination (Join-Path $INSTALL_DIR 'lib') -Recurse -Force
    }

    $srcBin = Join-Path $mingwRoot 'bin'
    if (Test-Path $srcBin) {
        Copy-Item -Path (Join-Path $srcBin '*.dll') -Destination (Join-Path $INSTALL_DIR 'bin') -Force
    }
}

# =============================
# Telechargement des archives MinGW officielles
# =============================
$SDL_ARCHIVE = "SDL2-devel-$SDL_VERSION-mingw.tar.gz"
$SDL_IMAGE_ARCHIVE = "SDL2_image-devel-$SDL_IMAGE_VERSION-mingw.tar.gz"
$SDL_TTF_ARCHIVE = "SDL2_ttf-devel-$SDL_TTF_VERSION-mingw.tar.gz"
$SDL_MIXER_ARCHIVE = "SDL2_mixer-devel-$SDL_MIXER_VERSION-mingw.tar.gz"

Download-IfMissing -FileName $SDL_ARCHIVE -Url "https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/$SDL_ARCHIVE"
Download-IfMissing -FileName $SDL_IMAGE_ARCHIVE -Url "https://github.com/libsdl-org/SDL_image/releases/download/release-$SDL_IMAGE_VERSION/$SDL_IMAGE_ARCHIVE"
Download-IfMissing -FileName $SDL_TTF_ARCHIVE -Url "https://github.com/libsdl-org/SDL_ttf/releases/download/release-$SDL_TTF_VERSION/$SDL_TTF_ARCHIVE"
Download-IfMissing -FileName $SDL_MIXER_ARCHIVE -Url "https://github.com/libsdl-org/SDL_mixer/releases/download/release-$SDL_MIXER_VERSION/$SDL_MIXER_ARCHIVE"

# =============================
# Installation locale dans client/SDL2
# =============================
Install-SdlPackage -ArchiveName $SDL_ARCHIVE -ExtractedDirName ("SDL2-$SDL_VERSION") -PrettyName 'SDL2'
Install-SdlPackage -ArchiveName $SDL_IMAGE_ARCHIVE -ExtractedDirName ("SDL2_image-$SDL_IMAGE_VERSION") -PrettyName 'SDL2_image'
Install-SdlPackage -ArchiveName $SDL_TTF_ARCHIVE -ExtractedDirName ("SDL2_ttf-$SDL_TTF_VERSION") -PrettyName 'SDL2_ttf'
Install-SdlPackage -ArchiveName $SDL_MIXER_ARCHIVE -ExtractedDirName ("SDL2_mixer-$SDL_MIXER_VERSION") -PrettyName 'SDL2_mixer'

# =============================
# Cleanup
# =============================
Set-Location $ROOT_DIR
Remove-Item -Path $BUILD_DIR -Recurse -Force

if (Get-Command git -ErrorAction SilentlyContinue) {
    $isGitRepo = git -C "$ROOT_DIR" rev-parse --is-inside-work-tree 2>$null
    if ($LASTEXITCODE -eq 0) {
        git -C "$ROOT_DIR" config core.hooksPath .githooks
        Write-Host 'Git hooks configured: core.hooksPath=.githooks'
    }
}

Write-Host ''
Write-Host '====================================================================='
Write-Host ' SDL2 + SDL2_image + SDL2_ttf + SDL2_mixer installes avec succes '
Write-Host ' (localement dans client/SDL2, sans MSYS2 obligatoire) '
Write-Host '====================================================================='
