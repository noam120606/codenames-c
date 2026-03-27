$ErrorActionPreference = 'Stop'

$port = 4242
$serverIp = if ($env:SERVER_IP) { $env:SERVER_IP } else { '127.0.0.1' }
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (-not (Test-Path './build')) {
    New-Item -ItemType Directory -Path './build' | Out-Null
}

$sdlCFlags = @()
$sdlLdFlags = @()

if (Get-Command pkg-config -ErrorAction SilentlyContinue) {
    $pkgCFlagsRaw = & pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer
    $pkgLdFlagsRaw = & pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer
    if ($LASTEXITCODE -eq 0) {
        $sdlCFlags = $pkgCFlagsRaw -split '\s+' | Where-Object { $_ -ne '' }
        $sdlLdFlags = $pkgLdFlagsRaw -split '\s+' | Where-Object { $_ -ne '' }
    }
}

if ($sdlCFlags.Count -eq 0 -or $sdlLdFlags.Count -eq 0) {
    if ((Test-Path './SDL2/include') -and (Test-Path './SDL2/lib')) {
        if ((Test-Path './SDL2/include/SDL.h') -and -not (Test-Path './SDL2/include/SDL2/SDL.h')) {
            New-Item -ItemType Directory -Force -Path './SDL2/include/SDL2' | Out-Null
            Copy-Item -Path './SDL2/include/*.h' -Destination './SDL2/include/SDL2' -Force -ErrorAction SilentlyContinue
        }
        $sdlCFlags = @('-I./SDL2/include')
        $sdlLdFlags = @('-L./SDL2/lib', '-lmingw32', '-lSDL2main', '-lSDL2', '-lSDL2_image', '-lSDL2_ttf', '-lSDL2_mixer')
    }
}

if ($sdlCFlags.Count -eq 0 -or $sdlLdFlags.Count -eq 0) {
    throw 'SDL2 libs not found. Install SDL2/SDL2_image/SDL2_ttf/SDL2_mixer (MSYS2) or provide ./SDL2/include and ./SDL2/lib.'
}

$localSdlBin = (Resolve-Path './SDL2/bin' -ErrorAction SilentlyContinue)
if ($localSdlBin) {
    $env:PATH = "$($localSdlBin.Path);$env:PATH"
}

$sources = Get-ChildItem -Path './src' -Filter '*.c' | ForEach-Object { $_.FullName }
if ($sources.Count -eq 0) {
    throw 'No client source files found in src/.'
}

$compileArgs = @()
$compileArgs += $sources
$compileArgs += @('-Wall', '-Wextra', '-O2', '-finput-charset=UTF-8', '-I./lib')
$compileArgs += $sdlCFlags
$compileArgs += @('-o', './build/client.exe')
$compileArgs += $sdlLdFlags
$compileArgs += @('-lws2_32', '-lm')

Write-Host 'Compiling client...'
& gcc @compileArgs
if ($LASTEXITCODE -ne 0) {
    throw "Client build failed with code $LASTEXITCODE"
}

Write-Host "Starting client against $serverIp`:$port..."
& ./build/client.exe -s $serverIp -p $port
