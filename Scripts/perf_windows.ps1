param(
    [string]$ExePath = '',
    [int[]]$Resolutions = @(1920, 1600),
    [int]$TimeoutSeconds = 150,
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$OutputJson = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$projectRoot = (Resolve-Path $ProjectRoot).Path
$workDir = Join-Path $projectRoot 'work'
New-Item -ItemType Directory -Force -Path $workDir | Out-Null
if (-not $ExePath) {
    $ExePath = Join-Path $projectRoot 'Saved\StagedBuilds\Windows\AuraPlayground\Binaries\Win64\AuraPlayground.exe'
}
$bin = (Resolve-Path $ExePath).Path
$projDir = Split-Path (Split-Path (Split-Path $bin -Parent) -Parent) -Parent
$resultFile = Join-Path $projDir 'Saved\FpsAudit.json'

$map = '/Game/PiedmontRide/Maps/PiedmontWorld'
$common = @('-game', '-windowed', '-ForceRes', '-BattleSkipTutorial', '-unattended', '-nosound', '-stdout', '-BattleFpsAudit')

$results = @()
foreach ($res in $Resolutions) {
    $height = [math]::Round($res * 9 / 16)
    $log = Join-Path $workDir "perf-$res.log"
    if (Test-Path $log) { [System.IO.File]::Delete($log) }
    if (Test-Path $resultFile) { [System.IO.File]::Delete($resultFile) }
    $runArgs = @($map) + @("-ResX=$res") + @("-ResY=$height") + $common
    $proc = Start-Process -FilePath $bin -ArgumentList $runArgs -RedirectStandardOutput $log -RedirectStandardError ($log + '.err') -PassThru
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Seconds 10
        if (Test-Path $resultFile) { break }
    }
    if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force }
    Start-Sleep -Seconds 3
    $found = $null
    if (Test-Path $resultFile) {
        $found = Get-Content -Path $resultFile -Raw -ErrorAction SilentlyContinue
    }
    if (-not $found) {
        $m = Select-String -Path $log -Pattern 'BattleFpsAudit: ' -ErrorAction SilentlyContinue | Select-Object -Last 1
        if ($m) { $found = $m.Line }
    }
    if ($found) {
        $avg = if ($found -match '"avg_fps":([0-9.]+)') { [double]$Matches[1] } else { 0 }
        $min = if ($found -match '"min_fps":([0-9.]+)') { [double]$Matches[1] } else { 0 }
        $p05 = if ($found -match '"p05_fps":([0-9.]+)') { [double]$Matches[1] } else { 0 }
        $n   = if ($found -match '"samples":(\d+)') { [int]$Matches[1] } else { 0 }
        $result = [pscustomobject]@{ exe = Split-Path $bin -Leaf; width = $res; height = $height; samples = $n; min_fps = $min; avg_fps = $avg; p05_fps = $p05; max_fps = $null; source = 'BattleFpsAudit' }
    } else {
        $result = [pscustomobject]@{ exe = Split-Path $bin -Leaf; width = $res; height = $height; samples = 0; note = 'no BattleFpsAudit result captured' }
    }
    $results += $result
    $result | Format-List | Out-String | Write-Output
}

if ($OutputJson) {
    $results | ConvertTo-Json -Depth 4 | Set-Content -Path $OutputJson -Encoding UTF8
    Write-Output "wrote $OutputJson"
}