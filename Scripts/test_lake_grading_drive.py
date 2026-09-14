"""Drive a candidate lake path in both directions with normal keyboard input."""
import argparse, json, re, subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--real',action='store_true');p.add_argument('--path',default='61853018');p.add_argument('--lookahead',type=int);p.add_argument('--brake-bends',action='store_true');args=p.parse_args()
root=Path(__file__).resolve().parents[1];mode=('real' if args.real else 'arcade')+(f'-lookahead{args.lookahead}' if args.lookahead else '')+('-braked' if args.brake_bends else '');log=root/f'work/lake-drive-{args.path}-{mode}.log'
cmd=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontLakePathGradingReview?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-unattended','-nosound','-RCWebControlDisable','-BattleConnectorAudit',f'-BattleParkHillPath={args.path}','-ExecCmds=t.IdleWhenNotForeground 0','-stdout']
if args.real:cmd.append('-BattleRealHandlingRoute')
if args.brake_bends:cmd.append('-BattleRouteBrakeForBends')
if args.lookahead:cmd.append(f'-BattleRouteLookahead={args.lookahead}')
with log.open('w') as f:
 try:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=240).returncode
 except subprocess.TimeoutExpired:code=-1
text=log.read_text(errors='replace');rows=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',text);report=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True};report.update(exit_code=code,mode=mode,path=args.path,scope='Uncooked candidate map, two-direction keyboard traversal. Not visual or packaged acceptance.');report['passed']=report['passed'] and code==0
(root/f'Tests/Results/2026-09-13-lake-drive-{args.path}-{mode}.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
raise SystemExit(0 if report['passed'] else 1)
