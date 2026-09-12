"""Exercise both Irwin/Lake road lanes across repaired junction joins."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/pothole-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattlePotholeAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'PotholeAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Native pothole contact-decision fixture only. Authored visual depressions, actual riding traversal, placement and packaged behavior unverified.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results/2026-09-12-native-pothole-contact.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
