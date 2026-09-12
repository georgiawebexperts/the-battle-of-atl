"""Capture the retargeted recovery candidates; does not accept production recovery."""
import json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'work/player-recovery-runtime'/uuid.uuid4().hex
out.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 result=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattlePlayerRecoveryReview',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
text=(out/'run.log').read_text()
rows=re.findall(r'PlayerRecoveryReview: (\{[^\n]+\})',text)
images=[out/f'getup-{c}-{i}.png' for c in range(4) for i in range(1,5)]
r={'exit_code':result.returncode,'poses':[json.loads(row) for row in rows],'images':[str(p) for p in images],'capture_passed':result.returncode==0 and len(rows)==4 and all(p.is_file() for p in images),'scope':'Four retargeted clips on Ellison; no physical transition or production integration. Visual acceptance remains separate.'}
r['standing_height_passed']=len(r['poses'])==4 and all(130 < p['head_height'] < 200 and -2 < p['left_foot_height'] < 10 and -2 < p['right_foot_height'] < 10 for p in r['poses'])
(root/'Tests/Results/2026-09-12-player-recovery-runtime.json').write_text(json.dumps(r,indent=2)+'\n')
print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] and r['standing_height_passed'] else 1)
