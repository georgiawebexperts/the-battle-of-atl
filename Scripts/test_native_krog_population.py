"""Exercise automatic Krog traffic, queuing and endpoint recycling."""
import json,pathlib,re,subprocess,argparse
parser=argparse.ArgumentParser();parser.add_argument('--world',action='store_true');args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1];suffix='krog-world-population' if args.world else 'krog-population';log=root/f'work/{suffix}-audit.log'
map_name='PiedmontKrogWorldReview' if args.world else 'PiedmontKrogPopulationReview'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),f'/Game/PiedmontRide/Maps/{map_name}?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-BattleSkipTutorial','-BattleTrafficPopulationAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=200)
rows=re.findall(r'TrafficPopulationAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Automatic director on the real lanes and occupied-area yield control, with controlled offscreen observation. Yield proof is in the separate occupancy test; no forced wait required here. Rider interaction, visible traffic motion and packaged behavior pending.'};r['map']=map_name;r['other_corridor_directors_present']=args.world;r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/f'Tests/Results/2026-09-12-native-{suffix}.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
