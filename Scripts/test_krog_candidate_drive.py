"""Drive a candidate connection using native movement and keyboard input."""
import json,re,subprocess,argparse,hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--report',default='2026-09-12-krog-candidate-drive.json');parser.add_argument('--level-floor',action='store_true');parser.add_argument('--continuous-shell',action='store_true');parser.add_argument('--buildings',action='store_true');parser.add_argument('--finish-route',action='store_true');parser.add_argument('--rail',action='store_true');parser.add_argument('--world',action='store_true');parser.add_argument("--crowds",action="store_true");args=parser.parse_args()
assert sum([args.level_floor,args.continuous_shell,args.buildings,args.rail,args.world])<=1
assert Path(args.report).name==args.report
app=Path('/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd')
variant = 'world' if args.world else 'rail' if args.rail else 'buildings' if args.buildings else 'continuous-shell' if args.continuous_shell else 'level-floor' if args.level_floor else 'candidate'
route = 'finish-drive' if args.finish_route else 'drive'
log = root / f'work/krog-{variant}-{route}.log'
map_name = ('PiedmontKrogWorldReview' if args.world else 'PiedmontKrogRailReview' if args.rail else 'PiedmontKrogBuildingsReview' if args.buildings else
            'PiedmontKrogShellReview' if args.continuous_shell else
            'PiedmontKrogFloorReview' if args.level_floor else 'PiedmontKrogRoadReview')
with log.open('w') as stream:
    run=subprocess.run([str(app),str(root/'AuraPlayground.uproject'),f'/Game/PiedmontRide/Maps/{map_name}?Difficulty=Easy?AutoStart=1','-game','-BattleSkipTutorial','-RCWebControlDisable',
        '-nullrhi','-unattended','-nosound','-BattleHomeDriveAudit' if args.finish_route else '-BattleKrogAudit','-stdout']+(['-BattleKeepCrowds'] if args.crowds else []),stdout=stream,stderr=subprocess.STDOUT,timeout=600)
matches=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
ground=re.findall(r'TrailGroundAudit: samples=(\d+) paved=(\d+)',log.read_text())
result['ground_samples']={'total':int(ground[-1][0]),'paved':int(ground[-1][1])} if ground else {}
lights=re.findall(r'TunnelLightAudit: lit=(\d+) unlit=(\d+)',log.read_text())
result['lights']={'lit_samples':int(lights[-1][0]),'unlit_samples':int(lights[-1][1])} if lights else {}
traffic=re.findall(r'TrafficRideAudit: (\{[^\n]+\})',log.read_text())
result['traffic_observed']=json.loads(traffic[-1]) if traffic else None
crowds=re.findall(r'CrowdRideAudit: (\{[^\n]+\})',log.read_text())
result['crowd_observed']=json.loads(crowds[-1]) if crowds else None
result['exit_code']=run.returncode
result['map']=map_name
if args.rail or args.world:result['rail_manifest_sha256']=hashlib.sha256((root/'SourceAssets/Terrain/KrogRailContext/deck-manifest.json').read_bytes()).hexdigest()
if args.buildings or args.rail or args.world:result['building_manifest_sha256']=hashlib.sha256((root/'SourceAssets/Terrain/KrogBuildings/manifest.json').read_bytes()).hexdigest()
result['passed']=bool(result.get('passed') and run.returncode==0)
result['scope']=('Krog southern exit along Wylie Street to 98 Estoria patio, both directions with real W/A/D input; crowds disabled; appearance and quest completion unverified.' if args.finish_route else 'Isolated Krog road candidate map: Irwin through the complete Krog tunnel, both directions using real W/A/D input and CharacterMovement; crowds disabled to isolate terrain/decks; appearance unverified.')
if args.world:result['scope']='Combined map: W/A/D ride on selected route with road traffic directors active. Pedestrian traffic disabled by existing route fixture; not a full mixed-crowd playthrough.'
if args.crowds:
    result['scope']='Selected bidirectional route with live cars and autonomous pedestrians retained. Test rider brakes for people ahead using Space; no steering avoidance or rendered performance evaluation.'
    result['passed']=bool(result['passed'] and result['crowd_observed'] and result['crowd_observed']['max_walking_people']>0 and result['crowd_observed']['nearby_samples']>0)
(root/'Tests/Results'/args.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
