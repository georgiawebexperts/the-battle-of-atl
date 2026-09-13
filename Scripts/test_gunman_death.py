"""Native gunman collapse, pose following and dead-shooter suppression."""
import pathlib,subprocess,re,json,argparse,uuid,plistlib
parser=argparse.ArgumentParser();parser.add_argument('--render',action='store_true');parser.add_argument('--detailed-gunman',action='store_true');parser.add_argument('--app');parser.add_argument('--report');args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
capture=root/'work/gunman-death-review'/uuid.uuid4().hex
if args.app:
 bundle=plistlib.loads((pathlib.Path(args.app).parents[1]/'Info.plist').read_bytes())['CFBundleIdentifier']
 capture=pathlib.Path.home()/'Library/Containers'/bundle/'Data/Documents/BattleGunmanReview'/uuid.uuid4().hex
capture.mkdir(parents=True)
log=capture/"run.log"
with log.open('w') as f:
 run=subprocess.run([*([args.app] if args.app else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game',*(['-RenderOffscreen','-ResX=1280','-ResY=720','-ForceRes','-NoTextureStreaming',f'-GunmanReviewDir={capture}','-ExecCmds=r.ScreenshotDelegate 0'] if args.render else ['-nullrhi']),*(['-BattleDetailedGunman','-ini:Engine:[ConsoleVariables]:r.ShaderCompiler.JobCacheDDC=0'] if args.detailed_gunman else []),'-FixedSeed','-RCWebControlDisable','-BattleSkipTutorial','-BattleGunmanDeathAudit','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=300 if args.detailed_gunman else 90)
rows=re.findall(r'GunmanDeathAudit: (\{[^\n]+\})',log.read_text());r=json.loads(rows[-1]) if rows else {'passed':False,'reason':'Missing audit'}
r.update(log=str(log),detailed_gunman=args.detailed_gunman,app=args.app,exit_code=run.returncode,scope='Native fatal damage physics fixture; visible hip tracking, collapse, dead shooting suppression. Visual ground contact requires image review; not a full combat playtest.');r['passed']=r['passed'] and run.returncode==0
if args.detailed_gunman:
 r['detailed_body_loaded']='DetailedGunman: body=f_tal_nrw_body parts=4 physics=f_tal_nrw_body_skmesh_Physics' in log.read_text()
 r['passed']=r['passed'] and r['detailed_body_loaded']
if args.render:
 r['images']=[str(capture/name) for name in ['falling.png','settled.png']];r['passed']=r['passed'] and all(pathlib.Path(p).is_file() for p in r['images'])
(root/'Tests/Results'/(args.report or ('2026-09-12-gunman-death-render.json' if args.render else '2026-09-12-gunman-death.json'))).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
