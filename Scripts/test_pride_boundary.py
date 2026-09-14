"""Check native closure relocation, market blocking and notice position."""
from pathlib import Path
import subprocess,re,json
root=Path(__file__).resolve().parents[1];log=root/'work/pride-boundary-runtime.log';cmd=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-unattended','-nosound','-RCWebControlDisable','-BattleTutorialAudit','-BattlePrideBoundaryAudit','-BattleMarketClosureReview','-stdout']
with log.open('w') as f:
 try:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=120).returncode
 except subprocess.TimeoutExpired:code=-1
text=log.read_text(errors='replace');matches=re.findall(r'BattleTutorialAudit: (\{[^\n]+\})',text);r=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True};r['passed']=r['passed'] and code==0;r.update(exit_code=code,scope='Runtime fences, cleared old cutoff, market gate and coming-soon notice; not a full moving gate approach');(root/'Tests/Results/2026-09-13-pride-boundary.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
