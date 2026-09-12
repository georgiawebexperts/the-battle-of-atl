"""Render moving Krog cars approaching the crossing under a forced hold."""
import pathlib,subprocess,json,uuid,argparse,re
parser=argparse.ArgumentParser();parser.add_argument('--report',default='2026-09-12-krog-traffic-render.json');args=parser.parse_args();assert pathlib.Path(args.report).name==args.report
root=pathlib.Path(__file__).resolve().parents[1];out=root/'work/krog-traffic-review'/uuid.uuid4().hex;out.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogCrossingReview?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleMonroeReview','-BattleTrafficReviewInterval=8','-BattleTireContactReview','-BattleRoadLaneAudit','-BattleMonroeLanes','-RCWebControlDisable',f'-BattleHUDReviewDir={out}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
images=[out/f'monroe-{i}.png' for i in [1,2,3]];r={'exit_code':run.returncode,'images':[str(p) for p in images],'capture_passed':run.returncode==0 and all(p.is_file() for p in images) and 'MonroeReview: complete' in (out/'run.log').read_text(),'visual_accepted':False,'scope':'Actual native car movement, fixed approach cameras and wide view with forced crossing hold. No visible traffic signals are implied; not a rider playthrough or production timing test.'};(root/'Tests/Results'/args.report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r))
if not r['capture_passed']:raise SystemExit(1)

contacts=[json.loads(t) for t in re.findall(r'TireContactReview: (\{[^\n]+\})',(out/'run.log').read_text())]
contact_report={'samples':contacts,'passed':len(contacts)==24 and all(t['samples']>0 and t['missing']==0 and -1.0<=t['minimum_gap_cm']<=1.5 for t in contacts),'scope':'LOD0 visible lower tire vertices against native road in three fixed stopped-car snapshots. Not dynamic full-route tire acceptance.'}
(root/'Tests/Results/2026-09-12-krog-tire-contact.json').write_text(json.dumps(contact_report,indent=2)+'\n')
print(json.dumps({'tire_contact_passed':contact_report['passed'],'snapshots':len(contacts)}))
if not contact_report['passed']:raise SystemExit(1)
