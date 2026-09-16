"""Verify two guaranteed, ahead-of-player bench-fire encounters per run."""
import argparse,json,plistlib,re,subprocess,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--editor',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
staged=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';exe=staged/'Contents/MacOS/AuraPlayground';entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(exe)]
log=root/f'work/bench-encounters-{uuid.uuid4().hex}.log'
with log.open('w') as stream:run=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleAmbientBenchAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
m=re.findall(r'AmbientBenchAudit: (\{[^\n]+\})',log.read_text());checks=json.loads(m[-1]) if m else None;result={'passed':bool(run.returncode==0 and checks and checks.get('passed')),'checks':checks,'exit_code':run.returncode,'editor':a.editor,'log':str(log),'scope':'Two guaranteed per-run encounters, quiet period, ahead placement, ignition and exact two-completion cap.'};Path(a.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));raise SystemExit(0 if result['passed'] else 1)
