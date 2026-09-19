param(
    [string]$Map = '/Game/PiedmontRide/Maps/PiedmontCarLaneReview',
    [string]$EditorPath = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe',
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot)
)

# BattleRoadLaneAudit drives two authored opposing car lanes to their endpoints.
# The lane-review actors are tagged TenthCarLaneReview and live in
# Content/PiedmontRide/Maps/PiedmontCarLaneReview, not in PiedmontWorld, and that
# review map is deliberately not cooked into the share, so this audit cannot run
# from a packaged build. It runs from the editor, against the review map:
#
#     powershell -ExecutionPolicy Bypass -File Scripts\run_road_lane_audit.ps1
#
# Windows mirror of Scripts/run_road_lane_audit.sh (Mac). Last known good there:
# 2 cars, total_distance_cm 81399.055, "Both opposing road lanes traversed with
# wheel support and endpoint stops".

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$projectRoot = (Resolve-Path $ProjectRoot).Path
$uproject = Join-Path $projectRoot 'AuraPlayground.uproject'
$workDir = Join-Path $projectRoot 'work'
New-Item -ItemType Directory -Force -Path $workDir | Out-Null
$log = Join-Path $workDir 'road-lane-audit.log'

if (-not (Test-Path $EditorPath)) {
    throw "UnrealEditor.exe not found at $EditorPath"
}

$runArgs = @(
    $uproject,
    "${Map}?Difficulty=Easy?AutoStart=1",
    '-game', '-RenderOffscreen', '-windowed', '-ResX=640', '-ResY=360', '-ForceRes',
    '-BattleSkipTutorial', '-unattended', '-nosound', '-stdout',
    '-BattleRoadLaneAudit'
)

& $EditorPath @runArgs 2>&1 | Out-File -Encoding utf8 -FilePath $log

$verdict = Select-String -Path $log -Pattern 'RoadLaneAudit: ' | Select-Object -Last 1
if ($verdict -and $verdict.Line -match '"passed":\s*true') {
    Write-Output "road lanes: pass  $($verdict.Line.Trim())"
    exit 0
}
Write-Output "road lanes: FAIL $(if ($verdict) { $verdict.Line.Trim() } else { '<no verdict>' })  (log: $log)"
exit 1
