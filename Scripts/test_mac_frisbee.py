"""Exercise real park groups and ammo supplies in the cooked Mac app."""
import argparse,csv,json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy');p.add_argument('--report',default='2026-09-11-build026-native-frisbee.json');args=p.parse_args();assert Path(args.report).name==args.report
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
log=root/('work/mac-frisbee-'+args.difficulty.lower()+'.log')
with log.open('w') as stream:
 run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty='+args.difficulty+'?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleFrisbeeAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=100)
matches=re.findall(r'BattleFrisbeeAudit: (\{[^\n]+\})',log.read_text());result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
row=next(r for r in csv.DictReader((root/'SourceAssets/Data/difficulty.csv').open()) if r['Name']==args.difficulty)
grass=re.findall(r'FrisbeeGrassRide: distance=([0-9.]+) groundedGrassSeconds=([0-9.]+) peakSpeed=([0-9.]+) wipeouts=([0-9]+)',log.read_text())
result['grass_ride']=dict(zip(['distance_cm','grounded_grass_seconds','peak_speed_cm_s','wipeouts'],map(float,grass[-1]))) if grass else {}
result.update(exit_code=run.returncode,difficulty=args.difficulty,expected_groups=int(row['FrisbeeGroups']),physical_input_and_audio_verified=False)
result['passed']=bool(result.get('passed') and result.get('groups')==result['expected_groups'] and run.returncode==0)
(root/'Tests/Results'/args.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
