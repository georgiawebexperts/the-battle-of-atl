"""Inspect the mapped signal from a westbound approach camera."""
import pathlib,json,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/native-signal-review'/uuid.uuid4().hex;out.mkdir(parents=True)
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with (out/'run.log').open('w') as log:
 r=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontSignalCrossingReview?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleSignalApproachReview',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
report={'exit_code':r.returncode,'images':[str(out/name) for name in ['signal-approach.png']],'capture_passed':r.returncode==0 and all((out/name).is_file() for name in ['signal-approach.png']) and 'TrafficSignalApproachReview: complete' in (out/'run.log').read_text(),'visual_accepted':False,'scope':'Native approach view from the mapped westbound car lane in the isolated signal crossing world'}
(root/'Tests/Results/2026-09-12-native-signal-approach.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['capture_passed'] else 1)
