"""Cooked native weapon pickup, combat and persistent inventory checks."""
import json,re,subprocess,argparse,csv
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--report',default='2026-09-11-native-inventory.json');p.add_argument('--difficulty',default='Easy',choices=['Easy','Medium','Hard']);a=p.parse_args();assert Path(a.report).name==a.report
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground';log=root/'work/mac-inventory.log'
with log.open('w') as f:
 run=subprocess.run([str(app),f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleInventoryAudit','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleInventoryAudit: (\{[^\n]+\})',log.read_text());result=json.loads(m[-1]) if m else {'passed':False,'missing_report':True}
expected=next(int(row['WeaponCrates']) for row in csv.DictReader((root/'SourceAssets/Data/Difficulty.csv').open()) if row['Name']==a.difficulty)
result['difficulty']=a.difficulty;result['expected_crates']=expected;result['passed']=bool(result.get('passed') and result.get('crates')==expected)
result['exit_code']=run.returncode;result['passed']=bool(result.get('passed') and run.returncode==0);result['rendered_appearance_verified']=False
(root/'Tests/Results'/a.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
