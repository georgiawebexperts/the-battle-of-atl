"""Native current-map shoreline ride, swimming and parked-bike return."""
import argparse,json,re,subprocess,uuid
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--review',action='store_true');p.add_argument('--detailed-rider',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
r=Path(__file__).resolve().parents[1];out=r/'work'/('swim-'+uuid.uuid4().hex);out.mkdir()
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleSwimReviewDir={out}'] if a.review else ['-nullrhi']
if a.detailed_rider:flags+=['-BattleDetailedRider']
if a.review and a.detailed_rider:flags+=['-BattleFootBodyReview']
with (out/'run.log').open('w') as f:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(r/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-BattleSwimAudit','-BattleSkipTutorial','-unattended','-nosound','-stdout',*flags],stdout=f,stderr=subprocess.STDOUT,timeout=120)
log=(out/'run.log').read_text();m=re.findall(r'BattleSwimAudit: (\{[^\n]+\})',log);d=json.loads(m[-1]) if m else {'passed':False,'reason':'Missing native report'};d.update(exit_code=run.returncode,detailed=a.detailed_rider,log=str(out/'run.log'),images=[str(x) for x in out.glob('*.png')]);d['passed']=d['passed'] and run.returncode==0
assert Path(a.report).name==a.report;(r/'Tests/Results'/a.report).write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d));raise SystemExit(0 if d['passed'] else 1)
