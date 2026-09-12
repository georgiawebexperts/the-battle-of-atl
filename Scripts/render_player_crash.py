"""Capture the mounted-pose physics candidate; does not accept production recovery."""
import argparse,json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--recovery',action='store_true');parser.add_argument('--bike',action='store_true');parser.add_argument('--double-road',action='store_true');parser.add_argument('--solid-support',action='store_true');parser.add_argument('--free-joints',action='store_true');parser.add_argument('--pose-mirror',action='store_true');parser.add_argument('--speed',type=int,default=300);parser.add_argument('--side',type=int,choices=[-1,1],default=1);args=parser.parse_args()
out=root/'work/player-crash-runtime'/uuid.uuid4().hex
out.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 result=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattlePlayerCrashReview',f'-BattleCrashSpeed={args.speed}',f'-BattleCrashSide={args.side}',*(['-BattlePlayerRecoveryBlend'] if args.recovery else []),*(['-BattleFallenBikeReview'] if args.bike else []),*(['-BattleCrashDoubleRoad'] if args.double_road else []),*(['-BattleCrashSolidSupport'] if args.solid_support else []),*(['-BattleCrashFreeJoints'] if args.free_joints else []),*(['-BattleCrashPoseMirror'] if args.pose_mirror else []),f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
text=(out/'run.log').read_text()
rows=re.findall(r'PlayerCrashReview: (\{[^\n]+\})',text)
images=[out/f'player-fall-{i}.png' for i in range(1,5)]
skin=re.findall(r'CrashSkin: (\{[^\n]+\})',text)
if args.recovery:images += [out/f'player-recovery-{i}.png' for i in range(1,7)]
r={'exit_code':result.returncode,'launch_speed_cm_s':args.speed,'launch_side':args.side,'skin':json.loads(skin[-1]) if skin else None,'physics':json.loads(rows[-1]) if rows else None,'images':[str(p) for p in images],'capture_passed':result.returncode==0 and bool(rows) and all(p.is_file() for p in images),'scope':'Mounted-pose candidate physics only. No production trigger, bike fall, recovery, or visual acceptance.'}
r['actual_shape_diagnostics']=re.findall(r'CrashActualShape: ([^\n]+)',text)
r['skin_clearance_passed']=bool(r['skin']) and r['skin']['vertices']>0 and r['skin']['below_vertices']==0 and r['skin']['min_clearance_cm']>=-1
if args.recovery:
 transfer=re.findall(r'PlayerRecoveryTransfer: (\{[^\n]+\})',text);complete=re.findall(r'PlayerRecoveryComplete: (\{[^\n]+\})',text)
 r['transfer']=json.loads(transfer[-1]) if transfer else None;r['recovery']=json.loads(complete[-1]) if complete else None
 r['scope']='Mounted fall, pose transfer and get-up review; no production trigger, bike fall or final visual acceptance.'
if args.bike:
 bike=re.findall(r'FallenBikeReview: (\{[^\n]+\})',text);r['bike']=json.loads(bike[-1]) if bike else None;r['scope']='Combined player and bike fall with aligned recovery; no production input, camera, remount or final visual acceptance.'
report='2026-09-12-player-pose-mirror.json' if args.pose_mirror else '2026-09-12-player-free-joints.json' if args.free_joints else '2026-09-12-player-solid-support.json' if args.solid_support else '2026-09-12-player-bike-double-road.json' if args.double_road else '2026-09-12-player-bike-fall.json' if args.bike else '2026-09-12-player-fall-recovery.json' if args.recovery else '2026-09-12-player-crash-runtime.json'
if args.speed!=300 or args.side!=1:report=report.replace('.json',f'-{args.speed}-{args.side}.json')
(root/'Tests/Results'/report).write_text(json.dumps(r,indent=2)+'\n')
print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] and r['skin_clearance_passed'] and r['physics']['passed'] and (not args.recovery or (r.get('recovery') or {}).get('passed')) and (not args.bike or (r.get('bike') or {}).get('passed')) else 1)
