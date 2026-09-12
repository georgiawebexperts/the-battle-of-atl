"""Render the bench fire candidate against actual runtime park furniture."""
import pathlib,subprocess,json,uuid,sys
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/bench-fire-review'/uuid.uuid4().hex;out.mkdir(parents=True)
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with (out/'run.log').open('w') as log:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleBenchFireReview',*(['-BattleBenchFireSolidReview'] if '--solid' in sys.argv else []),f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
images=[out/f'fire-{i}.png' for i in range(1,4)]
r={'solid_diagnostic':'--solid' in sys.argv,'exit_code':run.returncode,'images':[str(p) for p in images],'captured':all(p.is_file() for p in images),'visual_accepted':False,'scope':'Native visual candidate on first actual bench; no rare-event logic, ignition NPC, damage or packaged acceptance'}
r['capture_passed']=run.returncode==0 and r['captured'] and 'BenchFireReview: captured=3 fire=1' in (out/'run.log').read_text()
(root/'Tests/Results/2026-09-12-native-bench-fire-render.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] else 1)
