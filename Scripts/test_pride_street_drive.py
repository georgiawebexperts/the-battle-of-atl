"""Keyboard-driven round trip across the candidate Piedmont/10th junction."""
from pathlib import Path
import argparse,json,re,subprocess,uuid,plistlib,shutil
p=argparse.ArgumentParser();p.add_argument('--app',type=Path);p.add_argument('--main',action='store_true');p.add_argument('--require-road-cleanup',action='store_true');p.add_argument('--report');p.add_argument('--real',action='store_true');p.add_argument('--gate',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--lookahead',type=int,default=180);args=p.parse_args();root=Path(__file__).resolve().parents[1];mode=('real' if args.real else 'arcade')+('-rendered' if args.review else '')+f'-lookahead{args.lookahead}'+('-gate' if args.gate else '')+('-main' if args.main or args.app else '')+('-packaged' if args.app else '');log=root/f'work/pride-drive-{mode}.log';capture=root/'work/pride-drive-captures'/uuid.uuid4().hex
cmd=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview?Difficulty=Easy?AutoStart=1','-game','-unattended','-nosound','-RCWebControlDisable','-BattleConnectorAudit','-BattleParkHillPath='+('pride-gate-review' if args.gate else 'pride-turn-review'),f'-BattleRouteLookahead={args.lookahead}','-BattleRouteBrakeForBends','-BattleRouteTrace','-ExecCmds=t.IdleWhenNotForeground 0','-stdout']
if args.main or args.app:
 assert args.gate,'Main-map verification uses the exported full-route fixture'
 cmd[2]=cmd[2].replace('PiedmontPrideStreetReview','PiedmontWorld');fixture=root/'Tests/Fixtures/pride-gate-route.json'
 if args.app:
  bid=plistlib.loads((args.app.parent.parent/'Info.plist').read_bytes())['CFBundleIdentifier'];documents=Path.home()/'Library/Containers'/bid/'Data/Documents';fixture_out=documents/'PrideRouteFixtures'/uuid.uuid4().hex;fixture_out.mkdir(parents=True);shutil.copy2(fixture,fixture_out/'route.json');fixture=fixture_out/'route.json';capture=documents/'PrideRouteReview'/uuid.uuid4().hex;cmd=[str(args.app.resolve())]+cmd[2:]
 cmd.append(f'-BattleRouteFixtureFile={fixture}')
cmd+=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes','-NoTextureStreaming',f'-BattleRouteCaptureDir={capture}'] if args.review else ['-nullrhi']
if args.real:cmd.append('-BattleRealHandlingRoute')
with log.open('w') as f:
 try:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=420).returncode
 except subprocess.TimeoutExpired:code=-1
text=log.read_text(errors='replace');matches=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',text);report=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True};report.update(exit_code=code,mode=mode,map='PiedmontWorld' if args.main or args.app else 'PiedmontPrideStreetReview',packaged=bool(args.app),captures=[str(p) for p in sorted(capture.glob('*.png'))],scope='Candidate map, ordinary W/A/D and braking, both directions. Does not test tutorial boundary or release.');report['passed']=report['passed'] and code==0
if args.gate:
 report['scope']='Candidate full street-to-gate round trip with real keyboard input, pavement gate and exactly one timer start; not packaged.';report['gate_starts']=text.count('BattleTutorial: gate crossed; countdown=3');report['passed']=report['passed'] and report['gate_starts']==1
if args.main:report['scope']='Editor executable on the main map: full street-to-gate round trip, ordinary keyboard input, pavement checks and exactly one gate start; not packaged.'
if args.app:report['scope']='Packaged main-map round trip, ordinary keyboard input, pavement checks and exactly one gate start; known visual polish remains.'
if args.main or args.app:report['passed']=report['passed'] and 'RouteFixtureFile: points=' in text
if args.review:report['passed']=report['passed'] and len(report['captures'])==4
if args.require_road_cleanup:
 cleanup=re.findall(r'PrideRoadCleanup: removed=(\d+) remaining=(\d+) success=(\d+)',text)
 report['road_cleanup']=list(map(int,cleanup[-1])) if cleanup else None
 report['passed']=report['passed'] and report['road_cleanup']==[48,366,1]
(root/'Tests/Results'/(args.report or f'2026-09-13-pride-drive-{mode}.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
