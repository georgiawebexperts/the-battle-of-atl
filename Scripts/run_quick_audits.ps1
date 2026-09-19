param(
    [string]$ExePath = '',
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$EditorPath = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe',
    [string]$WorkDir = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$projectRoot = (Resolve-Path $ProjectRoot).Path
$workDir = if ($WorkDir) { $WorkDir } else { Join-Path $projectRoot 'work' }
New-Item -ItemType Directory -Force -Path $workDir | Out-Null

$uproject = Join-Path $projectRoot 'AuraPlayground.uproject'
$map = '/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1'
$common = @('-game', '-RenderOffscreen', '-windowed', '-ResX=640', '-ResY=360', '-ForceRes', '-BattleSkipTutorial', '-unattended', '-nosound', '-stdout')

if ($ExePath) {
    $bin = (Resolve-Path $ExePath).Path
    $prefix = @($map)
    $suffix = @('-installed')
} else {
    $bin = $EditorPath
    if (-not (Test-Path $bin)) {
        throw "UnrealEditor.exe not found at $bin"
    }
    $prefix = @($uproject, $map)
    $suffix = @()
}

$audits = @(
    [pscustomobject]@{ Name = 'BattleCurseAudit';    Label = 'BattleCurseAudit';        Flag = '' },
    [pscustomobject]@{ Name = 'BattleTreeRideAudit'; Label = 'BattleTreeRideAudit';     Flag = '' },
    [pscustomobject]@{ Name = 'BattleTreeRideAudit'; Label = 'Realistic-BattleTreeRideAudit'; Flag = 'BattleRealTreeAudit' },
    [pscustomobject]@{ Name = 'BattleTroubleAudit';  Label = 'BattleTroubleAudit';      Flag = '' },
    [pscustomobject]@{ Name = 'BattleSteeringAudit'; Label = 'BattleSteeringAudit';     Flag = '' },
    [pscustomobject]@{ Name = 'BattleSleeperAudit';  Label = 'BattleSleeperAudit';      Flag = '' },
    [pscustomobject]@{ Name = 'BattleGrassAudit';    Label = 'BattleGrassAudit';        Flag = '' },
    [pscustomobject]@{ Name = 'BattleBikeCarAudit';  Label = 'BattleBikeCarAudit';      Flag = '' },
    [pscustomobject]@{ Name = 'BattleMurderKAudit';  Label = 'BattleMurderKAudit';      Flag = '' },
    [pscustomobject]@{ Name = 'BattlePickupAudit';   Label = 'BattlePickupAudit';       Flag = '' },
    [pscustomobject]@{ Name = 'BattleSpiritAudit';   Label = 'BattleSpiritAudit';       Flag = '' },
    [pscustomobject]@{ Name = 'BattleKrogCrashAudit'; Label = 'BattleKrogCrashAudit';   Flag = '' },
    [pscustomobject]@{ Name = 'BattleDuckAudit';     Label = 'BattleDuckAudit';         Flag = '' },
    [pscustomobject]@{ Name = 'BattleSpeedAudit';    Label = 'BattleSpeedAudit';        Flag = '' },
    [pscustomobject]@{ Name = 'BattleTimeAudit';     Label = 'BattleTimeAudit';         Flag = '' },
    [pscustomobject]@{ Name = 'BattleDiscAudit';     Label = 'BattleDiscAudit';         Flag = '' },
    [pscustomobject]@{ Name = 'BattleDiagonalAudit'; Label = 'BattleDiagonalAudit';     Flag = '' },
    [pscustomobject]@{ Name = 'BattleMusicAudit';    Label = 'BattleMusicAudit';        Flag = ''; Audio = $true },
    # End-to-end guard on the ending: walks the whole authored course and requires
    # the win to commit at the patio. This is the audit that would have caught a
    # rider reaching the party with no result screen.
    [pscustomobject]@{ Name = 'BattlePatioAudit';   Label = 'BattlePatioAudit';       Flag = '' }
)

function Invoke-Audit {
    param([string]$AuditName, [string]$AuditFlag, [string]$Log, [switch]$Audio)
    $runArgs = @($prefix) + @($common)
    if ($Audio) { $runArgs = @($runArgs | Where-Object { $_ -ne '-nosound' }) }
    if ($AuditName) { $runArgs += "-$AuditName" }
    if ($AuditFlag) { $runArgs += "-$AuditFlag" }
    $runArgs += @($suffix)
    & $bin @runArgs 2>&1 | Out-File -Encoding utf8 -FilePath $Log
}

$failed = 0
foreach ($audit in $audits) {
    $log = Join-Path $workDir "$($audit.Label).log"
    $audio = if ($audit.PSObject.Properties['Audio']) { [bool]$audit.PSObject.Properties['Audio'].Value } else { $false }
    Invoke-Audit -AuditName $audit.Name -AuditFlag $audit.Flag -Log $log -Audio:$audio
    $verdict = Select-String -Path $log -Pattern '"passed":' | Select-Object -Last 1
    if (-not $verdict) {
        Invoke-Audit -AuditName $audit.Name -AuditFlag $audit.Flag -Log $log -Audio:$audio
        $verdict = Select-String -Path $log -Pattern '"passed":' | Select-Object -Last 1
    }
    if ($verdict -and $verdict.Line -match '"passed":\s*true') {
        Write-Output "-- $($audit.Label): pass"
    } else {
        $reason = if ($verdict) { $verdict.Line } else { '<no verdict>' }
        Write-Output "-- $($audit.Label): FAIL $reason"
        $failed = 1
    }
}

exit $failed
