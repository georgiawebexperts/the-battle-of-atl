"""Render the bench fire candidate against actual runtime park furniture."""
import pathlib,subprocess,json,uuid,sys,re
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/bench-fire-review'/uuid.uuid4().hex;out.mkdir(parents=True)
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with (out/'run.log').open('w') as log:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleBenchFireReview',*(['-BattleBenchIgnitionReview'] if '--ignite' in sys.argv else []),*(['-BattleBenchReachReview'] if '--reach' in sys.argv else []),*(['-BattleBenchFireSolidReview'] if '--solid' in sys.argv else []),f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
images=[out/f'fire-{i}.png' for i in range(1,4)]
r={'ignition_diagnostic':'--ignite' in sys.argv,'reach_diagnostic':'--reach' in sys.argv,'solid_diagnostic':'--solid' in sys.argv,'exit_code':run.returncode,'images':[str(p) for p in images],'captured':all(p.is_file() for p in images),'visual_accepted':False,'scope':'Native six-second fade/expiry visual candidate on first actual bench; no rare-event logic, ignition NPC, damage or packaged acceptance'}
if '--reach' in sys.argv:r['scope']='Native reach playback beside first actual bench; no ignition, prop, natural trigger or packaged acceptance'
r['capture_passed']=run.returncode==0 and r['captured'] and 'BenchFireReview: captured=3 expired=1' in (out/'run.log').read_text()
if '--reach' in sys.argv:
 text=(out/'run.log').read_text()
 r['playback_completed']='BenchReachReview: started=1' in text and 'sample=3 reaching=0' in text
 r['capture_passed']=r['capture_passed'] and r['playback_completed']
if '--ignite' in sys.argv:
 text=(out/'run.log').read_text()
 r['scope']='Combined native character reach and delayed ignition; prop contact, natural trigger and packaged acceptance pending'
 positions=re.findall(r'BenchIgnitionReview: sample=(\d) ignited=\d visitor_local=X=([-\d.]+) Y=([-\d.]+) Z=([-\d.]+)',text)
 r['visitor_positions']={row[0]:[float(v) for v in row[1:]] for row in positions}
 r['retreat_verified']='1' in r['visitor_positions'] and '3' in r['visitor_positions'] and r['visitor_positions']['3'][1]-r['visitor_positions']['1'][1]>100
 r['capture_passed']=r['capture_passed'] and 'sample=1 ignited=0' in text and 'sample=3 ignited=1' in text and 'sample=3 reaching=0' in text and r['retreat_verified']
(root/('Tests/Results/2026-09-12-native-bench-ignition-render.json' if '--ignite' in sys.argv else 'Tests/Results/2026-09-12-native-bench-reach-render.json' if '--reach' in sys.argv else 'Tests/Results/2026-09-12-native-bench-fire-render.json')).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['capture_passed'] else 1)
