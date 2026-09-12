"""Ride beside and across the authored shallow pothole through normal keyboard input."""
import argparse,json,pathlib,re,subprocess
parser=argparse.ArgumentParser();parser.add_argument("--main",action="store_true");args=parser.parse_args()
map_name="PiedmontWorld" if args.main else "PiedmontPotholeVisualReview"
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/pothole-ride-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),f'/Game/PiedmontRide/Maps/{map_name}?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-BattleSkipTutorial','-BattlePotholeRideAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'PotholeRideAudit: (\{[^\n]+\})',log.read_text());r={'map':map_name,'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Native keyboard riding beside and across one authored shallow pothole. Test contact actor reset between passes; no traffic, deep-hole, jump, visual-speed or packaged acceptance.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/('Tests/Results/2026-09-12-native-pothole-main-ride.json' if args.main else 'Tests/Results/2026-09-12-native-pothole-ride.json')).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
