"""Verify Mac release-facing focus pause and display options."""
import argparse,json,pathlib,re,shutil,subprocess,uuid
p=argparse.ArgumentParser();p.add_argument('--packaged',action='store_true');p.add_argument('--report',default='2026-09-16-mac-storefront-editor.json');a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1];run_id=uuid.uuid4().hex;out=root/'work'/f'storefront-{run_id}';out.mkdir(parents=True);log=out/'run.log'
capture=pathlib.Path.home()/'Library/Containers/com.webexperts.battleofatl/Data/Documents/BattleStorefrontReview'/run_id if a.packaged else out;capture.mkdir(parents=True,exist_ok=True)
entry=[str(root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground')] if a.packaged else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
cmd=[*entry,'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-BattleSkipTutorial','-BattleStorefrontAudit',f'-BattleStorefrontReviewDir={capture}','-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-RCWebControlDisable','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout']
with log.open('w') as stream:r=subprocess.run(cmd,stdout=stream,stderr=subprocess.STDOUT,timeout=60)
if a.packaged and (capture/'options.png').is_file():shutil.copy2(capture/'options.png',out/'options.png')
rows=re.findall(r'BattleStorefrontAudit: (\{[^\n]+\})',log.read_text());report=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True}
report.update(exit_code=r.returncode,packaged=a.packaged,image=str(out/'options.png'),scope='Native focus-loss pause helper, Unreal user-settings resolution/fullscreen state, and rendered Options menu. Physical macOS Space switching remains a manual check.')
report['passed']=bool(report.get('passed') and r.returncode==0 and (out/'options.png').is_file())
(root/'Tests/Results'/a.report).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
