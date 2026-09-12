"""Inspect imported car with actual game lighting; no traffic or map edits."""
import pathlib,json,subprocess,uuid,sys
driving="--driving" in sys.argv
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/native-car-review'/uuid.uuid4().hex;out.mkdir(parents=True)
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with (out/'run.log').open('w') as log:
 r=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleCarReview',*(['-BattleDrivingCarReview'] if driving else []),f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
report={'exit_code':r.returncode,'image':str(out/'car.png'),'capture_passed':r.returncode==0 and (out/'car.png').is_file() and 'CarReview: complete' in (out/'run.log').read_text(),'visual_accepted':False,'scope':('Native moving-car frame; continuous animation, traffic population, crossings and packaged behavior unverified' if driving else 'Native static car candidate; traffic, wheel contact, collision and packaged behavior unverified')}
(root/('Tests/Results/2026-09-12-native-driving-car-render.json' if driving else 'Tests/Results/2026-09-12-native-car-render.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['capture_passed'] else 1)
