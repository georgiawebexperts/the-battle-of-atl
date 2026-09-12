"""Exercise actual keyboard bike/car collision, recovery and remount or death."""
import argparse,json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--death',action='store_true');args=p.parse_args()
out=root/'work/player-crash-live'/uuid.uuid4().hex;out.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 result=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattlePlayerCrashAudit',*(['-BattleCrashDeathInterrupt'] if args.death else []),f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
text=(out/'run.log').read_text();rows=re.findall(r'PlayerCrashAudit: (\{[^\n]+\})',text)
r={'exit_code':result.returncode,'audit':json.loads(rows[-1]) if rows else None,'log':str(out/'run.log'),'images':[str(x) for x in sorted(out.glob('*.png'))],'events':re.findall(r'PlayerCrash(?:Live|Foot): ([^\n]+)',text),'scope':'One keyboard collision fixture and live recovery/remount or death interruption. Full visuals, arbitrary terrain and other hazards not accepted.'}
r['passed']=result.returncode==0 and bool(r['audit']) and r['audit']['passed']
(root/'Tests/Results'/('2026-09-12-player-crash-live-death.json' if args.death else '2026-09-12-player-crash-live.json')).write_text(json.dumps(r,indent=2)+'\n')
print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
