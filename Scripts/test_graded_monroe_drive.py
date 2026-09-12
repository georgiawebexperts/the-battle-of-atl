"""Exercise the isolated graded Monroe map with real W/A/D and bike movement."""
import subprocess,json,re
from pathlib import Path
root=Path(__file__).resolve().parents[1];log=root/'work/graded-monroe-drive.log'
with log.open('w') as stream:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontGradedDrive?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-unattended','-nosound','-RCWebControlDisable','-BattleConnectorAudit','-ExecCmds=t.IdleWhenNotForeground 0','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',log.read_text());result=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True};result['exit_code']=run.returncode;result['passed']=result.get('passed',False) and run.returncode==0;result['scope']='Uncooked native game world, isolated Monroe crossing, both directions via real keyboard and bike movement. Does not prove all road edges or packaged build.'
(root/'Tests/Results/2026-09-11-graded-monroe-drive.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
