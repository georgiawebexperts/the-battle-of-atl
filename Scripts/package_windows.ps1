param(
    [string]$Build = '115',
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$ArchiveRoot = 'D:\Battle Of ATL\Archives\BattleForTheA',
    [string]$BuildConfig = 'Development'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$uproject = Join-Path $projectRoot 'AuraPlayground.uproject'
$runUat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$workDir = Join-Path $projectRoot 'work'
$log = Join-Path $workDir 'windows-package.log'

if (-not (Test-Path $runUat)) {
    throw "RunUAT.bat not found at $runUat"
}
New-Item -ItemType Directory -Force -Path $workDir | Out-Null
New-Item -ItemType Directory -Force -Path $ArchiveRoot | Out-Null

# UAT's -archive merges into an existing archive tree and leaves stale files
# behind (old Engine folder, previous configs). Start from a clean archive
# target so the share prep step never picks up stale artifacts.
$archiveTarget = Join-Path $ArchiveRoot 'Windows'
if (Test-Path $archiveTarget) {
    Remove-Item -LiteralPath $archiveTarget -Recurse -Force
}

$cookDirs = "$projectRoot\Content\CitySampleCrowd+$projectRoot\Content\BattleRetarget+$projectRoot\Content\EuropeanHornbeam+$projectRoot\Content\BattleForTheA\Spirit+$projectRoot\Content\BattleForTheA\Environment\KrogIncident"
$buildArgs = @(
    'BuildCookRun',
    "-project=$uproject",
    '-noP4',
    '-platform=Win64',
    "-clientconfig=$BuildConfig",
    '-build',
    '-cook',
    '-map=/Game/PiedmontRide/Maps/PiedmontWorld',
    "-CookDir=$cookDirs",
    '-stage',
    '-pak',
    '-archive',
    "-archivedirectory=$ArchiveRoot",
    '-utf8output',
    '-unattended'
)

& $runUat @buildArgs > $log 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "Windows package failed with exit code $LASTEXITCODE. Log: $log"
}

Write-Output "Windows Development build $Build archived under $ArchiveRoot"
