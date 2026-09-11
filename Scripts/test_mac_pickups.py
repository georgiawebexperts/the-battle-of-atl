"""Exercise real pickup layouts and collection/health interactions for each profile."""
import json,re,subprocess,csv
from pathlib import Path
root=Path(__file__).resolve().parents[1];results=[]
expected={r['Name']:int(r['HealthPickups']) for r in csv.DictReader((root/'SourceAssets/Data/Difficulty.csv').open())}
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
for name in ['Easy','Medium','Hard']:
 log=root/'work'/('mac-pickup-'+name.lower()+'.log')
 with log.open('w') as stream:
  run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty='+name+'?AutoStart=1','-nullrhi','-unattended','-nosound','-BattlePickupAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
 matches=re.findall(r'BattlePickupAudit: (\{[^\n]+\})',log.read_text());row=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
 row['exit_code']=run.returncode;row['expected_pickups']=expected[name];row['passed']=bool(row.get('passed') and run.returncode==0 and row.get('spawned')==expected[name]);results.append(row);print(json.dumps(row),flush=True)
(root/'Tests/Results/2026-09-11-native-cola-pickups.json').write_text(json.dumps({'profiles':results,'all_passed':all(r['passed'] for r in results),'scope':'Actual layouts and tick-driven collection, controlled damage/position/occlusion fixtures. Visual and audio acceptance pending.'},indent=2)+'\n')
if not all(r['passed'] for r in results):raise SystemExit(1)
