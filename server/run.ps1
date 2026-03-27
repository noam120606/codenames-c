$ErrorActionPreference = 'Stop'

$port = 4242
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (-not (Test-Path './build')) {
    New-Item -ItemType Directory -Path './build' | Out-Null
}

$sources = Get-ChildItem -Path './src' -Filter '*.c' | ForEach-Object { $_.FullName }
if ($sources.Count -eq 0) {
    throw 'No server source files found in src/.'
}

$compileArgs = @()
$compileArgs += $sources
$compileArgs += @('-Wall', '-Wextra', '-O2', '-I./lib', '-lws2_32', '-o', './build/server.exe')

Write-Host 'Compiling server...'
& gcc @compileArgs
$buildExitCode = $LASTEXITCODE

if ($buildExitCode -ne 0) {
    # Sous Windows, ld peut échouer avec "Permission denied" si server.exe est déjà en cours.
    $runningServer = Get-Process server -ErrorAction SilentlyContinue
    if ($runningServer) {
        Write-Host 'Detected running server.exe, stopping it and retrying build...'
        $runningServer | Stop-Process -Force
        Start-Sleep -Milliseconds 300
        & gcc @compileArgs
        $buildExitCode = $LASTEXITCODE
    }
}

if ($buildExitCode -ne 0) {
    throw "Server build failed with code $buildExitCode"
}

Write-Host "Starting server on port $port..."
& ./build/server.exe -p $port
