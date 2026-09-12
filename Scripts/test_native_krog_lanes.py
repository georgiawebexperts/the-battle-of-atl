"""Exercise both Krog/DeKalb road lanes across repaired junction joins."""
import argparse,json,pathlib,re,subprocess
parser=argparse.ArgumentParser()
parser.add_argument('--crowned',action='store_true',help='Test isolated crowned DeKalb road, without the other approach branches.')
args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
stem='dekalb-crowned' if args.crowned else 'krog-lane'
map_name='PiedmontDeKalbCrownedReview' if args.crowned else 'PiedmontKrogLaneReview'
log=root/f'work/{stem}-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),f'/Game/PiedmontRide/Maps/{map_name}?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-BattleSkipTutorial','-BattleRoadLaneAudit','-BattleMonroeLanes','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'RoadLaneAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Two opposing Krog/DeKalb lanes in the isolated lane review map. Native motion and support, no signals, natural population or packaged acceptance.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
r['map']=map_name
if args.crowned:r['scope']='Two opposing cars on the separate crowned road and cut terrain. Other candidate approach branches omitted. No full crossing, bike, visual or packaged acceptance.'
report='2026-09-12-native-dekalb-crowned.json' if args.crowned else '2026-09-12-native-krog-lanes.json'
(root/'Tests/Results'/report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
