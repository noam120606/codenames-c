$ErrorActionPreference = 'Stop'

$defaultIp = '127.0.0.1'
$serverIp = Read-Host "Entrez l'adresse IP du serveur (par defaut: $defaultIp)"
if ([string]::IsNullOrWhiteSpace($serverIp)) {
    $serverIp = $defaultIp
}

$env:SERVER_IP = $serverIp
& "$PSScriptRoot/run.ps1"
