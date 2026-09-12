"""Render the starting bungalow through the actual opening sequence in the editor game."""
import json,pathlib,subprocess,uuid,re
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/tutorial-house'/uuid.uuid4().hex;out.mkdir(parents=True)
with (out/'run.log').open('w') as stream:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleOpeningReview',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
images=[str(out/f'opening{i}.png') for i in range(3)];matches=re.findall(r'BattleOpeningAudit: (\{[^\n]+\})',(out/'run.log').read_text());audit=json.loads(matches[-1]) if matches else {}
r={'exit_code':run.returncode,'opening':audit,'capture_passed':run.returncode==0 and audit.get('passed',False) and all(pathlib.Path(p).is_file() for p in images),'images':images,'visual_review':'pending','scope':'Starting-house material and detail review during opening; no whole-neighborhood or traversal acceptance.'}
(root/'Tests/Results/2026-09-12-tutorial-house-render.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] else 1)
