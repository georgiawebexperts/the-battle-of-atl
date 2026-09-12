"""Check the placed ambient sleeper in its actual park review map."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
out=root/'work/ambient-sleeper-runtime-render';out.mkdir(parents=True,exist_ok=True)
log=out/'run.log'
with log.open('w') as stream:
 result=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontSleeperReview?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes','-BattleAmbientSleeperRender',f'-BattleHUDReviewDir={out}','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-BattleSkipTutorial','-BattleAmbientSleeperAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'BattleAmbientSleeperAudit: (\{[^\n]+\})',log.read_text())
report={'exit_code':result.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Uncooked review-map ambient BeginPlay, runtime wake/fall clearance and re-wake; runtime images captured for separate visual inspection; automatic proximity selection and pursuit not exercised'}
report['missing_images']=[name for name in ['sleeping.png','standing.png','landed.png'] if not (out/name).exists()]
report['passed']=not report['missing_images'] and result.returncode==0 and report['checks'] is not None and report['checks']['passed']
(root/'Tests/Results/2026-09-12-native-ambient-sleeper-render.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
