"""Inspect naturally spawned road cars in the main world."""
import pathlib,json,subprocess,uuid,re
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/native-traffic-review'/uuid.uuid4().hex;out.mkdir(parents=True)
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with (out/'run.log').open('w') as log:
 r=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleTrafficReview',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
report={'exit_code':r.returncode,'images':[str(out/name) for name in ['traffic-1.png','traffic-2.png','traffic-3.png']],'capture_passed':r.returncode==0 and all((out/name).is_file() for name in ['traffic-1.png','traffic-2.png','traffic-3.png']) and 'TrafficReview: complete' in (out/'run.log').read_text(),'visual_accepted':False,'scope':'Naturally spawned main-world traffic under a controlled review camera; rider input, performance and packaging remain unverified.'}
report['frames']=[json.loads(x) for x in re.findall(r'TrafficVisual: (\{[^\n]+\})',(out/'run.log').read_text())]
(root/'Tests/Results/2026-09-12-native-traffic-render.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['capture_passed'] else 1)
