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
    [pscustomobject]@{ Name = 'BattlePatioAudit';   Label = 'BattlePatioAudit';       Flag = '' },
    # The Krog Street Tunnel: bore length, hazard stations on the tunnel floor,
    # and a pothole that can actually throw the rider (mirrors the Mac sweep).
    [pscustomobject]@{ Name = 'BattleTunnelHazardAudit'; Label = 'BattleTunnelHazardAudit'; Flag = '' },
    # Entries added for parity with the Mac 30-entry sweep (their classification
    # pass found these outside the sweep, unblocking but unclassified on Windows).
    [pscustomobject]@{ Name = 'BattleMeleeAudit';     Label = 'BattleMeleeAudit';         Flag = '' },
    [pscustomobject]@{ Name = 'BattleTrailModeAudit'; Label = 'BattleTrailModeAudit';     Flag = '' },
    [pscustomobject]@{ Name = 'BattleSkaterAudit';    Label = 'BattleSkaterAudit';        Flag = '' },
    [pscustomobject]@{ Name = 'BattleDroneAudit';     Label = 'BattleDroneAudit';         Flag = '' },
    [pscustomobject]@{ Name = 'BattleFrisbeeAudit';   Label = 'BattleFrisbeeAudit';       Flag = '' },
    [pscustomobject]@{ Name = 'BattlePanicAudit';     Label = 'BattlePanicAudit';         Flag = '' },
    [pscustomobject]@{ Name = 'BattleSpareBikeAudit'; Label = 'BattleSpareBikeAudit';     Flag = '' },
    [pscustomobject]@{ Name = 'BattleTutorialAudit';  Label = 'BattleTutorialAudit';      Flag = ''; NoSkip = $true },
    [pscustomobject]@{ Name = 'BattleMarketImpactAudit'; Label = 'BattleMarketImpactAudit'; Flag = ''; NoSkip = $true },
    [pscustomobject]@{ Name = 'BattleScooterTrafficAudit'; Label = 'BattleScooterTrafficAudit'; Flag = 'BattleFurnitureAudit' }
    [pscustomobject]@{ Name = 'BattleStreakAudit'; Label = 'BattleStreakAudit'; Flag = '' }
)
# BATTLE_SWEEP_EXTENDED=1 adds the 39 audits Mac build 152 brought into the
# sweep (75 exist total; these had never run in any sweep). They get a tighter
# two-minute cap by default, because an untriaged audit that hangs should cost
# two minutes rather than seven.
$extended = ($env:BATTLE_SWEEP_EXTENDED -eq '1')
if ($extended) {
    $audits += @(
        [pscustomobject]@{ Name = 'BattleAimAudit'; Label = 'BattleAimAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleAmmoAudit'; Label = 'BattleAmmoAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleFinishAudit'; Label = 'BattleFinishAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleFootAudit'; Label = 'BattleFootAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleGeographyAudit'; Label = 'BattleGeographyAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleHealthAudit'; Label = 'BattleHealthAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleHornAudit'; Label = 'BattleHornAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleInventoryAudit'; Label = 'BattleInventoryAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleJumpAudit'; Label = 'BattleJumpAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleKnifeAudit'; Label = 'BattleKnifeAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleSkateAudit'; Label = 'BattleSkateAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleSkylineAudit'; Label = 'BattleSkylineAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleStorefrontAudit'; Label = 'BattleStorefrontAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleSwimAudit'; Label = 'BattleSwimAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleWatchAudit'; Label = 'BattleWatchAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleZombieAudit'; Label = 'BattleZombieAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleConnectorAudit'; Label = 'BattleConnectorAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleCrossingReservationAudit'; Label = 'BattleCrossingReservationAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleEntranceWalkAudit'; Label = 'BattleEntranceWalkAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleMonroeOccupancyAudit'; Label = 'BattleMonroeOccupancyAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattlePhoneRideAudit'; Label = 'BattlePhoneRideAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattlePlayerCrashAudit'; Label = 'BattlePlayerCrashAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattlePotholeAudit'; Label = 'BattlePotholeAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattlePotholeRideAudit'; Label = 'BattlePotholeRideAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleTrafficPopulationAudit'; Label = 'BattleTrafficPopulationAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleZombiePopulationAudit'; Label = 'BattleZombiePopulationAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleAmbientBenchAudit'; Label = 'BattleAmbientBenchAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleAmbientSleeperAudit'; Label = 'BattleAmbientSleeperAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleBenchFireAudit'; Label = 'BattleBenchFireAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleBenchIgnitionAudit'; Label = 'BattleBenchIgnitionAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleBenchReachAudit'; Label = 'BattleBenchReachAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleRoadAmberAudit'; Label = 'BattleRoadAmberAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleRoadCarAudit'; Label = 'BattleRoadCarAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleRoadCrossingAudit'; Label = 'BattleRoadCrossingAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleRoadLaneAudit'; Label = 'BattleRoadLaneAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleRoadTrafficAudit'; Label = 'BattleRoadTrafficAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleSleeperChaseAudit'; Label = 'BattleSleeperChaseAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleSleeperSettleAudit'; Label = 'BattleSleeperSettleAudit'; Flag = '' },
        [pscustomobject]@{ Name = 'BattleSleeperTriggerAudit'; Label = 'BattleSleeperTriggerAudit'; Flag = '' }
    )
    if (-not $env:BATTLE_AUDIT_TIMEOUT) { $env:BATTLE_AUDIT_TIMEOUT = '120' }
}


# BATTLE_SWEEP_ONLY=Name,Name runs just those entries, for verifying one audit
# without paying for the full launch list (mirrors the Mac sweep script).
$only = $env:BATTLE_SWEEP_ONLY
if ($only) {
    $wanted = @($only -split ',') | ForEach-Object { $_.Trim() } | Where-Object { $_ }
    $audits = @($audits | Where-Object { $wanted -contains $_.Name -or $wanted -contains $_.Label })
}

function Invoke-Audit {
    param([string]$AuditName, [string]$AuditFlag, [string]$Log, [switch]$Audio, [switch]$NoSkip)
    $runArgs = @($prefix) + @($common)
    if ($NoSkip) { $runArgs = @($runArgs | Where-Object { $_ -ne '-BattleSkipTutorial' }) }
    if ($Audio) { $runArgs = @($runArgs | Where-Object { $_ -ne '-nosound' }) }
    if ($AuditName) { $runArgs += "-$AuditName" }
    if ($AuditFlag) { $runArgs += "-$AuditFlag" }
    $runArgs += @($suffix)
    # Run with a hard timeout, mirroring the Mac sweep: an audit that prints its
    # verdict and then fails to exit is killed and still scored on its verdict.
    $errLog = "$Log.err"
    $proc = Start-Process -FilePath $bin -ArgumentList $runArgs -PassThru -RedirectStandardOutput $Log -RedirectStandardError $errLog
    if (-not $proc.WaitForExit($auditTimeoutSec * 1000)) {
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 2
    }
}

$auditTimeoutSec = if ($env:BATTLE_AUDIT_TIMEOUT) { [int]$env:BATTLE_AUDIT_TIMEOUT } else { 420 }
$settleSec = if ($env:BATTLE_SWEEP_SETTLE) { [int]$env:BATTLE_SWEEP_SETTLE } else { 3 }

$failed = 0
foreach ($audit in $audits) {
    # Let the machine settle between launches so the previous game teardown
    # cannot produce a false red on the next one (mirrors the Mac sweep).
    Start-Sleep -Seconds $settleSec
    $log = Join-Path $workDir "$($audit.Label).log"
    $audio = if ($audit.PSObject.Properties['Audio']) { [bool]$audit.PSObject.Properties['Audio'].Value } else { $false }
    $noskip = if ($audit.PSObject.Properties['NoSkip']) { [bool]$audit.PSObject.Properties['NoSkip'].Value } else { $false }
    Invoke-Audit -AuditName $audit.Name -AuditFlag $audit.Flag -Log $log -Audio:$audio -NoSkip:$noskip
    $verdict = Select-String -Path $log -Pattern '"passed":' | Select-Object -Last 1
    if (-not $verdict) {
        Invoke-Audit -AuditName $audit.Name -AuditFlag $audit.Flag -Log $log -Audio:$audio -NoSkip:$noskip
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
