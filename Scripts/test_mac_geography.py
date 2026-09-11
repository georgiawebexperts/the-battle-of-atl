"""Test cooked Mac geographic anchors, radar axes, island and timed water return."""
import json,re,subprocess,argparse
from pathlib import Path
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--report',default='2026-09-11-native-geography.json');args=parser.parse_args()
assert Path(args.report).name==args.report
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
log=root/'work/mac-geography.log'
with log.open('w') as stream:
    run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',
        '-nullrhi','-unattended','-nosound','-BattleGeographyAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
matches=re.findall(r'BattleGeographyAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
water=re.findall(r'GeographyWaterReturn: insideBoundary=(\d) onPath=(\d) onBridge=(\d) onDirt=(\d)',log.read_text())
result['water_return']=dict(zip(['inside_boundary','on_path','on_bridge','on_gravel'],[bool(int(v)) for v in water[-1]])) if water else {}
result['exit_code']=run.returncode
result['passed']=bool(result.get('passed') and run.returncode==0)
result['rendered_appearance_verified']=False
(root/'Tests/Results'/args.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
