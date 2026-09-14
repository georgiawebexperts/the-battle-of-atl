"""Keyboard-driven round trip across the candidate Piedmont/10th junction."""
from pathlib import Path
import argparse,json,re,subprocess,uuid
p=argparse.ArgumentParser();p.add_argument('--real',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--lookahead',type=int,default=180);args=p.parse_args();root=Path(__file__).resolve().parents[1];mode=('real' if args.real else 'arcade')+('-rendered' if args.review else '')+f'-lookahead{args.lookahead}';log=root/f'work/pride-drive-{mode}.log';capture=root/'work/pride-drive-captures'/uuid.uuid4().hex
cmd=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview?Difficulty=Easy?AutoStart=1','-game','-unattended','-nosound','-RCWebControlDisable','-BattleConnectorAudit','-BattleParkHillPath=pride-turn-review',f'-BattleRouteLookahead={args.lookahead}','-BattleRouteBrakeForBends','-BattleRouteTrace','-ExecCmds=t.IdleWhenNotForeground 0','-stdout']
cmd+=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes','-NoTextureStreaming',f'-BattleRouteCaptureDir={capture}'] if args.review else ['-nullrhi']
if args.real:cmd.append('-BattleRealHandlingRoute')
with log.open('w') as f:
 try:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=420).returncode
 except subprocess.TimeoutExpired:code=-1
text=log.read_text(errors='replace');matches=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',text);report=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True};report.update(exit_code=code,mode=mode,map='PiedmontPrideStreetReview',captures=[str(p) for p in sorted(capture.glob('*.png'))],scope='Candidate map, ordinary W/A/D and braking, both directions. Does not test tutorial boundary or release.');report['passed']=report['passed'] and code==0
if args.review:report['passed']=report['passed'] and len(report['captures'])==4
(root/f'Tests/Results/2026-09-13-pride-drive-{mode}.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
