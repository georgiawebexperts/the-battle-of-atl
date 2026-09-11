"""Validate actual spawned profiles and Hard's covered tunnel wave; freeze pursuit only."""
import json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];results=[]
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
for name in ['Easy','Medium','Hard']:
 log=root/'work'/('mac-zombie-population-'+name.lower()+'.log')
 with log.open('w') as stream:
  run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty='+name+'?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleZombiePopulationAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=150)
 matches=re.findall(r'BattleZombiePopulationAudit: (\{[^\n]+\})',log.read_text());row=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
 row['scope']='Configured population reached before completion; Hard then tests a full wave, a repeat wave and departure. Its final live count is after returning to the gate.'
 row['exit_code']=run.returncode;row['passed']=bool(row.get('passed') and run.returncode==0);results.append(row);print(json.dumps(row),flush=True)
(root/'Tests/Results/2026-09-11-native-zombie-populations.json').write_text(json.dumps({'profiles':results,'all_passed':all(r['passed'] for r in results),'scope':'Actual navigation/collision-checked spawns with pursuit frozen; tuning values and covered Hard wave, not rendered density or game balance.'},indent=2)+'\n')
if not all(r['passed'] for r in results):raise SystemExit(1)
