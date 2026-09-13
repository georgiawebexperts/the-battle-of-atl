"""Capture three native-moving-car views on each reviewed turnaround."""
import json,math,pathlib,subprocess,uuid,re,argparse
parser=argparse.ArgumentParser();parser.add_argument("--two-cascades",action="store_true");args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
lanes=json.loads((root/'SourceAssets/Terrain/KrogTraffic/car-lanes.json').read_text())['routes']
e=next(r['points_cm'] for r in lanes if r['name']=='eastbound');w=next(r['points_cm'] for r in lanes if r['name']=='westbound')
length=lambda p:sum(math.dist(a[:2],b[:2]) for a,b in zip(p,p[1:]))
arc=math.pi*200;shots=[length(e)+arc*f for f in [.2,.5,.8]]+[length(e)+arc+length(w)+arc*f for f in [.2,.5,.8]]
out=root/'work/krog-turnarounds'/uuid.uuid4().hex;out.mkdir(parents=True)
with (out/'run.log').open('w') as log:
    run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogTurnaroundReview?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleFurnitureAudit','-BattleCarLoopAudit',f'-BattleLoopRenderDir={out}','-BattleLoopShotDistances='+','.join(str(s) for s in shots),'-RCWebControlDisable','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-stdout']+(['-ExecCmds=r.Shadow.CSM.MaxCascades 2'] if args.two_cascades else []),stdout=log,stderr=subprocess.STDOUT,timeout=300)
images=[out/f'turn-{i}.png' for i in range(6)];rows=re.findall(r'CarLoopAudit: (\{[^\n]+\})',(out/'run.log').read_text());checks=json.loads(rows[-1]) if rows else None
j={'capture_passed':bool(run.returncode==0 and checks and checks['passed'] and all(p.is_file() for p in images)),'checks':checks,'images':[str(p) for p in images],'visual_accepted':False,'scope':'Attempts six moving-car snapshots across two turns; consult checks for loop completion. Visual review required; not motion-video or multiplayer/crowd acceptance.'}
j['two_cascades_override']=args.two_cascades
j['tire_contacts']=[json.loads(t) for t in re.findall(r'TurnTireContact: (\{[^\n]+\})',(out/'run.log').read_text())]
j['tire_contact_passed']=len(j['tire_contacts'])==24 and all(t['samples']>0 and t['missing']==0 and -1<=t['minimum_gap_cm']<=1.5 for t in j['tire_contacts'])
(root/'Tests/Results/2026-09-13-krog-turnaround-render.json').write_text(json.dumps(j,indent=2)+'\n');print(json.dumps(j));raise SystemExit(0 if j['capture_passed'] and j['tire_contact_passed'] else 1)
