"""Exercise finish guards and optional win presentation in the combined world."""
import argparse,json,re,subprocess,uuid
from pathlib import Path
parser=argparse.ArgumentParser();parser.add_argument('--difficulty',choices=['Easy','Hard'],default='Easy');parser.add_argument('--render',action='store_true');parser.add_argument('--celebration',action='store_true');parser.add_argument("--packaged",action="store_true");parser.add_argument("--report");args=parser.parse_args();args.render=args.render or args.celebration
root=Path(__file__).resolve().parents[1];out=root/'work/krog-world-finish'/uuid.uuid4().hex;out.mkdir(parents=True)
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
map_name='PiedmontWorld' if args.packaged else 'PiedmontKrogWorldReview'
entry=[str(root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground')] if args.packaged else [engine,str(root/'AuraPlayground.uproject')]
command=entry+[f'/Game/PiedmontRide/Maps/{map_name}?Difficulty={args.difficulty}?AutoStart=1','-game','-BattleSkipTutorial','-BattleFinishAudit','-RCWebControlDisable','-unattended','-nosound','-stdout']
command+=['-RenderOffscreen','-BattleFinishReview',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0'] if args.render else ['-nullrhi']
if args.celebration:command.append('-BattleCelebrationReview')
with (out/'run.log').open('w') as log:r=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'BattleFinishAudit: (\{[^\n]+\})',(out/'run.log').read_text());result=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True}
result.update(exit_code=r.returncode,difficulty=args.difficulty,map=map_name,packaged=args.packaged,scope='Native trigger/guard fixture using teleports for phone, checkpoints and ordered tunnel entry/exit. Easy mounted finish; Hard on-foot finish. Not a complete route playthrough. Audit-only save slot verified in BattleFinish.cpp.')
if args.render:result.update(image=str(out/'win.png'),visual_review='pending')
if args.celebration:result['celebration_images']=[str(out/n) for n in ['celebration.png','morgan.png','cheers.png','credits-title.png','credits-developer.png','credits-future.png','win.png']]
result['passed']=bool(result.get('passed') and r.returncode==0 and (not args.render or (out/'win.png').is_file()))
if args.celebration:result['passed']=result['passed'] and all(Path(p).is_file() for p in result['celebration_images'])
report_name=args.report or f'2026-09-12-krog-world-{"celebration" if args.celebration else "finish"}-{args.difficulty}.json'
assert Path(report_name).name==report_name
(root/'Tests/Results'/report_name).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));raise SystemExit(0 if result['passed'] else 1)
