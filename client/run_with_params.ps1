param(
    [int]$Fps = 0,
    [string]$ServerIp,
    [int]$Instances = 0,
    [int]$Port = 4242,
    [switch]$NoBuild
)

$ErrorActionPreference = 'Stop'

$defaultIp = '127.0.0.1'
if ([string]::IsNullOrWhiteSpace($ServerIp)) {
    $ServerIp = Read-Host "Entrez l'adresse IP du serveur (par defaut: $defaultIp)"
}
if ([string]::IsNullOrWhiteSpace($ServerIp)) {
    $ServerIp = $defaultIp
}

if ($Instances -le 0) {
    $instancesRaw = Read-Host "Nombre de clients a lancer (par defaut: 1)"
    if ([string]::IsNullOrWhiteSpace($instancesRaw)) {
        $Instances = 1
    } else {
        [int]$parsedInstances = 0
        if (-not [int]::TryParse($instancesRaw, [ref]$parsedInstances) -or $parsedInstances -lt 1) {
            throw 'Le nombre de clients doit etre un entier superieur ou egal a 1.'
        }
        $Instances = $parsedInstances
    }
}

if ([int]$Fps -le 0) {
    $fpsRaw = Read-Host "Nombre de FPS (frames per second) cible (par defaut: 60)"
    if ([string]::IsNullOrWhiteSpace($fpsRaw)) {
        $Fps = 60
    } else {
        [int]$parsedFps = 0
        if (-not [int]::TryParse($fpsRaw, [ref]$parsedFps) -or $parsedFps -le 0) {
            throw 'Le nombre de FPS doit etre un entier superieur a 0.'
        }
        $Fps = $parsedFps
    }
}

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (-not $NoBuild) {
    Write-Host 'Compilation unique avant lancement multi-clients...'
    & "$root/run.ps1" -ServerIp $ServerIp -Port $Port -Fps $Fps -BuildOnly
}

for ($i = 1; $i -le $Instances; $i++) {
    Write-Host "Lancement du client $i/$Instances..."
    Start-Process -FilePath 'powershell' -WorkingDirectory $root -ArgumentList @(
        '-NoExit',
        '-ExecutionPolicy',
        'Bypass',
        '-File',
        "$root/run.ps1",
        '-ServerIp',
        $ServerIp,
        '-Port',
        "$Port",
        '-Fps',
        "$Fps",
        '-NoBuild'
    ) | Out-Null
}

Write-Host "$Instances client(s) lance(s)."
