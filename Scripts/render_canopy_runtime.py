"""Native fixed-view canopy comparison; frame times are diagnostic, not shipping FPS acceptance."""
import argparse,json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--baseline',action='store_true');p.add_argument('--mixed',action='store_true');p.add_argument('--report');p.add_argument('--krog',action='store_true');args=p.parse_args();name='baseline' if args.baseline else 'main-mixed' if args.mixed else 'main-swap'
out=root/'work/canopy-runtime'/f'{name}-{uuid.uuid4().hex}';out.mkdir(parents=True)
map_name='PiedmontKrogWorldReview' if args.krog else 'PiedmontWorld'
assert not args.krog or args.baseline, 'Krog review uses saved scenery; pass --baseline'
with (out/'run.log').open('w') as log:
 result=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),f'/Game/PiedmontRide/Maps/{map_name}?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-RCWebControlDisable','-BattleSkipTutorial','-BattleCanopyReview',*(['-BattleKrogSceneryReview'] if args.krog else []),* ([] if args.baseline else ['-BattleDetailedCanopy']+(['-BattleMixedCanopy'] if args.mixed else [])),f'-BattleHUDReviewDir={out}','-windowed','-ResX=1920','-ResY=1080','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0,t.MaxFPS 0,r.VSync 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
text=(out/'run.log').read_text();frames=[json.loads(s) for s in re.findall(r'CanopyFrame: (\{[^\n]+\})',text)];images=[out/f'canopy-{i}.png' for i in [1,2,3]]
r={'exit_code':result.returncode,'capture_passed':result.returncode==0 and len(frames)==3 and all(p.is_file() for p in images) and 'CanopyRuntime: complete' in text,'images':[str(p) for p in images],'frames':frames,'baseline':args.baseline,'map':map_name,'transient_main_world_swap':not args.baseline,'resolution':[1920,1080],'scope':'Three fixed native-game cameras with active world, five seconds warmup then four seconds frame sampling per view. Editor executable, hidden stationary rider, no riding/collision/audio or sustained shipping performance acceptance.'}
report_name=args.report or f'2026-09-12-canopy-runtime-{name}.json'
assert pathlib.Path(report_name).name==report_name
(root/'Tests/Results'/report_name).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] else 1)
