"""Capture handling label in both modes and existing on-foot HUD."""
import argparse,json,pathlib,subprocess,uuid
parser=argparse.ArgumentParser();parser.add_argument('--report',default='2026-09-12-handling-hud.json');args=parser.parse_args()
assert pathlib.Path(args.report).name == args.report
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/handling-hud'/uuid.uuid4().hex;out.mkdir(parents=True)
rows=[]
for mode in ('arcade','realistic'):
 folder=out/mode;folder.mkdir()
 with (folder/'run.log').open('w') as stream:
  run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattleHUDReview',f'-BattleHUDReviewDir={folder}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout']+(['-BattleRealHandlingHUD'] if mode=='realistic' else []),stdout=stream,stderr=subprocess.STDOUT,timeout=120)
 rows.append({'mode':mode,'exit_code':run.returncode,'images':[str(folder/name) for name in ('bike.png','foot.png')],'completed':'HUDReview: requested bike and foot captures' in (folder/'run.log').read_text()})
r={'capture_passed':all(row['exit_code']==0 and row['completed'] and all(pathlib.Path(p).is_file() for p in row['images']) for row in rows),'runs':rows,'visual_review':'pending','scope':'1280x720 mounted arcade/realistic labels and on-foot HUD. No whole-game art acceptance.'}
(root/'Tests/Results'/args.report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] else 1)
