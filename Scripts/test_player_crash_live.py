"""Exercise actual keyboard bike/car collision, recovery and remount or death."""
import argparse,json,pathlib,re,subprocess,uuid,plistlib,shutil
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--spare',action='store_true');p.add_argument('--obstacle',choices=['tree','person','wall']);p.add_argument('--app',type=pathlib.Path);p.add_argument('--detailed-rider',action='store_true');p.add_argument('--report');p.add_argument('--lights',action='store_true');mode=p.add_mutually_exclusive_group();mode.add_argument('--crowd-block',action='store_true');mode.add_argument('--solid-block',action='store_true');mode.add_argument('--taser',action='store_true');mode.add_argument('--drone',action='store_true');mode.add_argument('--death',action='store_true');mode.add_argument('--repeat',action='store_true');mode.add_argument('--death-on-foot',action='store_true');args=p.parse_args()
out=root/'work/player-crash-live'/uuid.uuid4().hex;out.mkdir(parents=True)
launcher=[str(args.app)] if args.app else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
if args.app and not args.app.is_file():raise SystemExit(f'Missing executable: {args.app}')
if args.report and pathlib.Path(args.report).name!=args.report:raise SystemExit('Report must be a filename')
capture=out
if args.app:
 bundle=args.app.parent.parent
 bundle_id=plistlib.loads((bundle/'Info.plist').read_bytes())['CFBundleIdentifier']
 capture=pathlib.Path.home()/'Library/Containers'/bundle_id/'Data/Documents/BattleCrashReview'/out.name
 capture.mkdir(parents=True,exist_ok=True)
with (out/'run.log').open('w') as log:
 result=subprocess.run([*launcher,'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattlePlayerCrashAudit',*(['-BattleCrashSpareBike'] if args.spare else []),*(['-BattleCrashObstacle='+args.obstacle] if args.obstacle else []),*(['-BattleDetailedRider'] if args.detailed_rider else []),*(['-BattleCrashDeathInterrupt'] if args.death or args.death_on_foot else []),*(['-BattleCrashRepeat'] if args.repeat else []),*(['-BattleCrashDeathOnFoot'] if args.death_on_foot else []),*(['-BattleCrashCrowdBlock'] if args.crowd_block else []),*(['-BattleCrashSolidBlock'] if args.solid_block else []),*(['-BattleCrashLights'] if args.lights else []),*(['-BattleCrashTaser'] if args.taser else []),*(['-BattleCrashDrone'] if args.drone else []),f'-BattleHUDReviewDir={capture}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
if capture!=out:
 for picture in capture.glob('*.png'):shutil.copy2(picture,out/picture.name)
text=(out/'run.log').read_text();rows=re.findall(r'PlayerCrashAudit: (\{[^\n]+\})',text)
r={'exit_code':result.returncode,'audit':json.loads(rows[-1]) if rows else None,'log':str(out/'run.log'),'images':[str(x) for x in sorted(out.glob('*.png'))],'events':re.findall(r'PlayerCrash(?:Live|Foot|Cycle): ([^\n]+)',text),'scope':'One keyboard collision fixture and live recovery/remount or death interruption. Full visuals, arbitrary terrain and other hazards not accepted.'}
r['detailed_assets_loaded']='DetailedRiderPreview: body=m_tal_nrw_body outfit_parts=4' in text
r['detailed_rider_preview']=args.detailed_rider
r['runtime']='packaged' if args.app else 'editor-game'
r['scope']=('Late crowd obstruction during get-up' if args.crowd_block else 'Late solid obstruction during get-up' if args.solid_block else 'Police taser physical knockdown' if args.taser else 'Drone swept physical knockdown' if args.drone else 'Two same-world collisions and remounts' if args.repeat else 'Collision, get-up and on-foot death' if args.death_on_foot else 'Collision and mid-fall death' if args.death else 'Collision and remount')+'; fixture suppresses ambient damage until the explicit death probe. Full animation, arbitrary terrain and other hazards not accepted.'
r['obstacle']=args.obstacle;r['spare_after_crash']='BattleSpareMount: recovered_crash=1' in text
if args.obstacle:r['scope']='Keyboard collision with '+('actual pedestrian' if args.obstacle=='person' else args.obstacle+' collision proxy')+', harm, -10 seconds, physical recovery and remount. Authored tree instances have a separate world test; arbitrary terrain and continuous animation quality remain unverified.'
r['pose_fits']=[json.loads(x) for x in re.findall(r'PlayerRecoveryFit: (\{[^\n]+\})',text)]
r['space_checks']=[json.loads(x) for x in re.findall(r'RecoverySpaceAudit: (\{[^\n]+\})',text)]
r['light_events']=re.findall(r'FallenBikeLights[^:]*: ([^\n]+)',text)
r['capture_passed']=len(r['images'])==((6 if args.repeat else 2 if args.death else 3)+(3 if args.detailed_rider and not args.death else 0)+(1 if args.lights and not (args.death or args.death_on_foot) else 0))
r['pose_fit_nonregression']=all(x['heading_rms_cm']<0 or x['selected_rms_cm']<=x['heading_rms_cm']+.002 for x in r['pose_fits'])
r['requested_flow_covered']=(not (args.crowd_block or args.solid_block) or (len(r['space_checks'])==1 and r['space_checks'][0]['passed'])) and (not args.taser or bool(r['audit'] and r['audit']['reason'].startswith('Warned police taser'))) and (not args.drone or bool(r['audit'] and r['audit']['reason'].startswith('Warned drone')))
r['passed']=(not args.spare or r['spare_after_crash']) and (not args.detailed_rider or r['detailed_assets_loaded']) and r['requested_flow_covered'] and r['capture_passed'] and r['pose_fit_nonregression'] and result.returncode==0 and bool(r['audit']) and r['audit']['passed']
(root/'Tests/Results'/(args.report or ('2026-09-12-player-crash-crowd-space.json' if args.crowd_block else '2026-09-12-player-crash-solid-space.json' if args.solid_block else '2026-09-12-player-crash-live-lights.json' if args.lights else '2026-09-12-player-crash-live-taser.json' if args.taser else '2026-09-12-player-crash-live-drone.json' if args.drone else '2026-09-12-player-crash-live-repeat.json' if args.repeat else '2026-09-12-player-crash-live-death-on-foot.json' if args.death_on_foot else '2026-09-12-player-crash-live-death.json' if args.death else '2026-09-12-player-crash-live.json'))).write_text(json.dumps(r,indent=2)+'\n')
print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
