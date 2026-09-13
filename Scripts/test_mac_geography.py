"""Test cooked Mac geographic anchors, radar axes, island and persistent swimmer/bank separation."""
import json,re,subprocess,argparse
from pathlib import Path
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--editor',action='store_true');parser.add_argument('--report',default='2026-09-11-native-geography.json');args=parser.parse_args()
assert Path(args.report).name==args.report
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if args.editor else [str(app)]
log=root/'work/mac-geography.log'
with log.open('w') as stream:
    run=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',
        '-game','-RCWebControlDisable','-BattleSkipTutorial','-nullrhi','-unattended','-nosound','-BattleGeographyAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
matches=re.findall(r'BattleGeographyAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result['editor']=args.editor
result['exit_code']=run.returncode
result['passed']=bool(result.get('passed') and run.returncode==0)
result['rendered_appearance_verified']=False
(root/'Tests/Results'/args.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
