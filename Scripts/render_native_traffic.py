"""Inspect naturally spawned road cars in the main world."""
import pathlib,json,subprocess,uuid,re,argparse,plistlib,shutil
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/native-traffic-review'/uuid.uuid4().hex;out.mkdir(parents=True)
parser=argparse.ArgumentParser();parser.add_argument('--app',type=pathlib.Path);parser.add_argument('--report',default='2026-09-12-native-traffic-render.json');args=parser.parse_args();assert pathlib.Path(args.report).name==args.report
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
launch=[engine,str(root/'AuraPlayground.uproject')];capture=out
if args.app:
 assert args.app.is_file(),args.app
 launch=[str(args.app)]
 info=plistlib.loads((args.app.parents[1]/'Info.plist').read_bytes())
 capture=pathlib.Path.home()/'Library/Containers'/info['CFBundleIdentifier']/'Data/Documents/BattleTrafficReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 r=subprocess.run(launch+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleTrafficReview',f'-BattleHUDReviewDir={capture}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
if args.app:
 for source in capture.glob('traffic-*.png'):shutil.copy2(source,out/source.name)
report={'packaged':bool(args.app),'exit_code':r.returncode,'images':[str(out/name) for name in ['traffic-1.png','traffic-2.png','traffic-3.png']],'capture_passed':r.returncode==0 and all((out/name).is_file() for name in ['traffic-1.png','traffic-2.png','traffic-3.png']) and 'TrafficReview: complete' in (out/'run.log').read_text(),'visual_accepted':False,'scope':'Naturally spawned main-world traffic under a controlled review camera; rider input, performance and packaging remain unverified.'}
if args.app:report['scope']='Packaged main-world traffic under a controlled review camera; full rider playthrough and performance remain unverified.'
report['frames']=[json.loads(x) for x in re.findall(r'TrafficVisual: (\{[^\n]+\})',(out/'run.log').read_text())]
(root/'Tests/Results'/args.report).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['capture_passed'] else 1)
