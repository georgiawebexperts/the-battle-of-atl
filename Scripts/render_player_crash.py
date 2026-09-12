"""Capture the mounted-pose physics candidate; does not accept production recovery."""
import json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'work/player-crash-runtime'/uuid.uuid4().hex
out.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 result=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattlePlayerCrashReview',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
text=(out/'run.log').read_text()
rows=re.findall(r'PlayerCrashReview: (\{[^\n]+\})',text)
images=[out/f'player-fall-{i}.png' for i in range(1,5)]
r={'exit_code':result.returncode,'physics':json.loads(rows[-1]) if rows else None,'images':[str(p) for p in images],'capture_passed':result.returncode==0 and bool(rows) and all(p.is_file() for p in images),'scope':'Mounted-pose candidate physics only. No production trigger, bike fall, recovery, or visual acceptance.'}
(root/'Tests/Results/2026-09-12-player-crash-runtime.json').write_text(json.dumps(r,indent=2)+'\n')
print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] else 1)
