param(
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

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (-not $NoBuild) {
    Write-Host 'Compilation unique avant lancement multi-clients...'
    & "$root/run.ps1" -ServerIp $ServerIp -Port $Port -BuildOnly
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
        '-NoBuild'
    ) | Out-Null
}

Write-Host "$Instances client(s) lance(s)."
