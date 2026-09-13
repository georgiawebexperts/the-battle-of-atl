"""Cooked native weapon pickup, combat and persistent inventory checks."""
import json,re,subprocess,argparse,csv,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--editor',action='store_true');p.add_argument('--detailed-rider',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--report',default='2026-09-11-native-inventory.json');p.add_argument('--difficulty',default='Easy',choices=['Easy','Medium','Hard']);a=p.parse_args();assert Path(a.report).name==a.report
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground';out=root/'work'/('inventory-'+uuid.uuid4().hex);out.mkdir();log=out/'run.log'
assert not a.detailed_rider or a.editor
assert not a.review or a.editor
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'-game','-RCWebControlDisable'] if a.editor else [str(app)]
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleInventoryReviewDir={out}'] if a.review else ['-nullrhi']
if a.detailed_rider:flags+=['-BattleDetailedRider']
with log.open('w') as f:
 run=subprocess.run([*entry,f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1',*flags,'-unattended','-nosound','-BattleInventoryAudit','-BattleSkipTutorial','-ExecCmds=t.IdleWhenNotForeground 0','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleInventoryAudit: (\{[^\n]+\})',log.read_text());result=json.loads(m[-1]) if m else {'passed':False,'missing_report':True}
expected=next(int(row['WeaponCrates']) for row in csv.DictReader((root/'SourceAssets/Data/Difficulty.csv').open()) if row['Name']==a.difficulty)
result['difficulty']=a.difficulty;result['expected_crates']=expected;result['passed']=bool(result.get('passed') and result.get('crates')==expected)
result['exit_code']=run.returncode;result['passed']=bool(result.get('passed') and run.returncode==0);result['rendered_appearance_verified']=False;result['editor']=a.editor;result['detailed_rider']='DetailedFootPreview: body=m_tal_nrw_body' in log.read_text();result['log']=str(log);result['images']=[str(x) for x in out.glob('*.png')]
if result['detailed_rider']:
 result['detailed_assets_loaded']=all(x in log.read_text() for x in ['DetailedFootPreview: body=m_tal_nrw_body','DetailedArmsPreview: hands=SK_DetailedHands','DetailedM1911: loaded']);result['passed']=result['passed'] and result['detailed_assets_loaded']
(root/'Tests/Results'/a.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
