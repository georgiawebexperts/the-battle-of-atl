"""Native solid-wall escape using actual rider key input, in both handling modes."""
import argparse,json,re,subprocess,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--packaged',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
entry=[str(root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground')] if a.packaged else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
results=[]
for real in [False,True]:
 log=root/f'work/wall-recovery-{uuid.uuid4().hex}.log'
 with log.open('w') as f:
  run=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-RCWebControlDisable','-BattleSkipTutorial','-BattleSteeringAudit','-BattleWallRecoveryAudit','-unattended','-nosound','-stdout']+(['-BattleRealHandlingAudit'] if real else []),stdout=f,stderr=subprocess.STDOUT,timeout=90)
 matches=re.findall(r'WallRecoveryAudit: (\{[^\n]+\})',log.read_text())
 r=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
 r.update(mode='realistic' if real else 'arcade',log=str(log),exit_code=run.returncode);r['passed']=bool(r['passed'] and run.returncode==0);results.append(r)
report={'passed':all(r['passed'] for r in results),'packaged':a.packaged,'runs':results,'scope':'Actual key input and collision on isolated floor/wall fixture. Does not prove all world corners, slopes, or visual handling feel.'}
Path(a.report).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
