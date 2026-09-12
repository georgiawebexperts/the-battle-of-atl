"""Native gunman warning, cover, dodge and fatal-hit contract."""
import pathlib,subprocess,re,json,argparse,uuid
parser=argparse.ArgumentParser();parser.add_argument('--render',action='store_true');args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/gunman-death-audit.log'
capture=root/'work/gunman-death-review'/uuid.uuid4().hex
if args.render:capture.mkdir(parents=True)
with log.open('w') as f:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game',*(['-RenderOffscreen','-ResX=1280','-ResY=720','-ForceRes','-NoTextureStreaming',f'-GunmanReviewDir={capture}','-ExecCmds=r.ScreenshotDelegate 0'] if args.render else ['-nullrhi']),'-BattleSkipTutorial','-BattleGunmanDeathAudit','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'GunmanDeathAudit: (\{[^\n]+\})',log.read_text());r=json.loads(rows[-1]) if rows else {'passed':False,'reason':'Missing audit'}
r.update(exit_code=run.returncode,scope='Native fatal damage physics fixture; visible hip tracking, collapse, dead shooting suppression. Visual ground contact requires image review; not a full combat playtest.');r['passed']=r['passed'] and run.returncode==0
if args.render:
 r['images']=[str(capture/name) for name in ['falling.png','settled.png']];r['passed']=r['passed'] and all(pathlib.Path(p).is_file() for p in r['images'])
(root/'Tests/Results'/('2026-09-12-gunman-death-render.json' if args.render else '2026-09-12-gunman-death.json')).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
