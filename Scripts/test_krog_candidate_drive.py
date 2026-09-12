"""Drive the installed connection using real cooked movement and keyboard input."""
import json,re,subprocess,argparse
from pathlib import Path
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--report',default='2026-09-12-krog-candidate-drive.json');args=parser.parse_args()
assert Path(args.report).name==args.report
app=Path('/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd')
log=root/'work/krog-candidate-drive.log'
with log.open('w') as stream:
    run=subprocess.run([str(app),str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogRoadReview?Difficulty=Easy?AutoStart=1','-game','-BattleSkipTutorial','-RCWebControlDisable',
        '-nullrhi','-unattended','-nosound','-BattleKrogAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=600)
matches=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
ground=re.findall(r'TrailGroundAudit: samples=(\d+) paved=(\d+)',log.read_text())
result['ground_samples']={'total':int(ground[-1][0]),'paved':int(ground[-1][1])} if ground else {}
lights=re.findall(r'TunnelLightAudit: lit=(\d+) unlit=(\d+)',log.read_text())
result['lights']={'lit_samples':int(lights[-1][0]),'unlit_samples':int(lights[-1][1])} if lights else {}
result['exit_code']=run.returncode
result['passed']=bool(result.get('passed') and run.returncode==0)
result['scope']='Isolated Krog road candidate map: Irwin through the complete Krog tunnel, both directions using real W/A/D input and CharacterMovement; crowds disabled to isolate terrain/decks; appearance unverified.'
(root/'Tests/Results'/args.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
