"""Native gunman warning, cover, dodge and fatal-hit contract."""
import pathlib,subprocess,re,json,argparse,uuid,plistlib
parser=argparse.ArgumentParser();parser.add_argument('--render',action='store_true');parser.add_argument('--detailed-gunman',action='store_true');parser.add_argument('--app');parser.add_argument('--report');args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
capture=root/'work/gunman-review'/uuid.uuid4().hex
if args.app:
 bundle=plistlib.loads((pathlib.Path(args.app).parents[1]/'Info.plist').read_bytes())['CFBundleIdentifier']
 capture=pathlib.Path.home()/'Library/Containers'/bundle/'Data/Documents/BattleGunmanReview'/uuid.uuid4().hex
capture.mkdir(parents=True)
log=capture/'run.log'
with log.open('w') as f:
 run=subprocess.run([*([args.app] if args.app else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game',*(['-RenderOffscreen','-ResX=1280','-ResY=720','-ForceRes','-NoTextureStreaming',f'-GunmanReviewDir={capture}','-ExecCmds=r.ScreenshotDelegate 0'] if args.render else ['-nullrhi']),*(['-BattleDetailedGunman','-ini:Engine:[ConsoleVariables]:r.ShaderCompiler.JobCacheDDC=0'] if args.detailed_gunman else []),'-BattleSkipTutorial','-BattleGunmanAudit','-FixedSeed','-RCWebControlDisable','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=300 if args.detailed_gunman else 90)
rows=re.findall(r'GunmanAudit: (\{[^\n]+\})',log.read_text());r=json.loads(rows[-1]) if rows else {'passed':False,'reason':'Missing audit'}
r.update(detailed_gunman=args.detailed_gunman,log=str(log),fixed_seed=True,app=args.app,exit_code=run.returncode,scope='Native stationary encounter fixture; warnings/cover/locked-aim dodge/fatal hit. Natural spawn rarity, incoming-fire direction, animations and rendering not accepted.');r['passed']=r['passed'] and run.returncode==0
if args.detailed_gunman:
 r['detailed_body_loaded']='DetailedGunman: body=f_tal_nrw_body parts=4 physics=f_tal_nrw_body_skmesh_Physics' in log.read_text()
 r['passed']=r['passed'] and r['detailed_body_loaded']
if args.render:
 r['images']=[str(capture/name) for name in ['warning.png','behind-warning.png','covered-shot.png','visible-shooter.png']];r['passed']=r['passed'] and all(pathlib.Path(p).is_file() for p in r['images'])
(root/'Tests/Results'/(args.report or ('2026-09-12-gunman-render.json' if args.render else '2026-09-12-gunman-core.json'))).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
