"""Capture three native-moving-car views on each reviewed turnaround."""
import json,math,pathlib,subprocess,uuid,re
root=pathlib.Path(__file__).resolve().parents[1]
lanes=json.loads((root/'SourceAssets/Terrain/KrogTraffic/car-lanes.json').read_text())['routes']
e=next(r['points_cm'] for r in lanes if r['name']=='eastbound');w=next(r['points_cm'] for r in lanes if r['name']=='westbound')
length=lambda p:sum(math.dist(a[:2],b[:2]) for a,b in zip(p,p[1:]))
arc=math.pi*200;shots=[length(e)+arc*f for f in [.2,.5,.8]]+[length(e)+arc+length(w)+arc*f for f in [.2,.5,.8]]
out=root/'work/krog-turnarounds'/uuid.uuid4().hex;out.mkdir(parents=True)
with (out/'run.log').open('w') as log:
    run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogTurnaroundReview?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleFurnitureAudit','-BattleCarLoopAudit',f'-BattleLoopRenderDir={out}','-BattleLoopShotDistances='+','.join(str(s) for s in shots),'-RCWebControlDisable','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=300)
images=[out/f'turn-{i}.png' for i in range(6)];rows=re.findall(r'CarLoopAudit: (\{[^\n]+\})',(out/'run.log').read_text());checks=json.loads(rows[-1]) if rows else None
j={'capture_passed':bool(run.returncode==0 and checks and checks['passed'] and all(p.is_file() for p in images)),'checks':checks,'images':[str(p) for p in images],'visual_accepted':False,'scope':'Six moving-car snapshots across two turns; native full loop maintained. Visual review required; not motion-video or multiplayer/crowd acceptance.'}
(root/'Tests/Results/2026-09-13-krog-turnaround-render.json').write_text(json.dumps(j,indent=2)+'\n');print(json.dumps(j));raise SystemExit(0 if j['capture_passed'] else 1)
