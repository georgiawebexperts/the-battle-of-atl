"""Native current-map shoreline ride, swimming and parked-bike return."""
import argparse,json,re,subprocess,uuid
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--review',action='store_true');p.add_argument('--first-person',action='store_true');p.add_argument('--detailed-rider',action='store_true');p.add_argument('--deep-drop',action='store_true');p.add_argument('--taser',action='store_true');p.add_argument('--route');p.add_argument('--edge',type=int,default=0);p.add_argument('--map',default='/Game/PiedmontRide/Maps/PiedmontWorld');p.add_argument('--report',required=True);a=p.parse_args()
r=Path(__file__).resolve().parents[1];out=r/'work'/('swim-'+uuid.uuid4().hex);out.mkdir()
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleSwimReviewDir={out}'] if a.review else ['-nullrhi']
assert not (a.deep_drop and (a.route or a.taser))
if a.first_person:flags += ['-BattleSwimViewAudit']
if a.deep_drop:flags += ['-BattleSwimDeepDrop']
assert not (a.route and a.taser),'Use separate route and taser checks'
if a.taser:flags += ['-BattleSwimTaser']
if a.route:flags += ['-BattleSwimRoute='+str(Path(a.route).resolve())]
if a.detailed_rider:flags+=['-BattleDetailedRider']
if a.review and a.detailed_rider and not a.first_person:flags+=['-BattleFootBodyReview']
with (out/'run.log').open('w') as f:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(r/'AuraPlayground.uproject'),a.map+'?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-BattleSwimAudit',f'-BattleSwimEdge={a.edge}','-BattleSkipTutorial','-unattended','-nosound','-stdout',*flags],stdout=f,stderr=subprocess.STDOUT,timeout=900 if a.route else 120)
log=(out/'run.log').read_text();fixture=re.findall(r'BattleSwimFixture: edge=(\d+) dry=([^\n]+)',log);m=re.findall(r'BattleSwimAudit: (\{[^\n]+\})',log);d=json.loads(m[-1]) if m else {'passed':False,'reason':'Missing native report'};d.update(deep_drop=a.deep_drop,edge_used=int(fixture[-1][0]) if fixture else None,fixture=fixture[-1][1] if fixture else None,edge_requested=a.edge,map=a.map,exit_code=run.returncode,detailed=a.detailed_rider,log=str(out/'run.log'),images=[str(x) for x in out.glob('*.png')]);d['passed']=d['passed'] and run.returncode==0
d['first_person']=a.first_person
d['stop_diagnostics']=re.findall(r'SwimStopDiagnostic: ([^\n]+)',log)
d['view_diagnostics']=re.findall(r'BattleSwimView: ([^\n]+)',log)
d['bank_support_diagnostics']=re.findall(r'BattleBankSupport: ([^\n]+)',log)
assert Path(a.report).name==a.report;(r/'Tests/Results'/a.report).write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d));raise SystemExit(0 if d['passed'] else 1)
