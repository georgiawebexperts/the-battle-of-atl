"""Review each native traffic paint on the existing driving car assembly."""
import json,pathlib,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/car-palette'/uuid.uuid4().hex;out.mkdir(parents=True)
with (out/'run.log').open('w') as stream:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattleCarReview','-BattleDrivingCarReview','-BattleCarPaletteReview',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
images=[out/f'car-paint-{i}.png' for i in range(6)]
r={'exit_code':run.returncode,'images':[str(p) for p in images],'capture_passed':run.returncode==0 and all(p.is_file() for p in images) and 'CarReview: complete' in (out/'run.log').read_text(),'visual_review':'pending','scope':'Six paint variants on one existing vehicle body. No additional body styles or full traffic-population acceptance.'}
(root/'Tests/Results/2026-09-12-traffic-car-palette.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] else 1)
