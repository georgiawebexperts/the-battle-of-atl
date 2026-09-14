"""Compare exposure in native park/tunnel cameras without saving the map."""
import json,pathlib,subprocess,uuid,argparse
root=pathlib.Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--report',default='2026-09-14-crowd-colors.json');args=parser.parse_args();assert pathlib.Path(args.report).name==args.report
out=root/'work'/('native-exposure-'+uuid.uuid4().hex);out.mkdir();runs=[]
for scene in ['park']:
 for grade in ['colors']:
  folder=out/(scene+'-'+grade);folder.mkdir()
  command=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleHUDReview','-BattleCrowdVarietyReview',f'-BattleHUDReviewDir={folder}','-RCWebControlDisable','-unattended','-nosound','-ResX=1280','-ResY=720','-windowed','-stdout']
  if scene=='tunnel':command.append('-BattleTunnelExposureReview')
  if grade=='candidate':command.append('-BattleExposureCandidate')
  with (folder/'run.log').open('w') as f:r=subprocess.run(command,stdout=f,stderr=subprocess.STDOUT,timeout=120)
  images=[str(folder/(name+'.png')) for name in ['bike','foot']]
  runs.append({'scene':scene,'grade':grade,'exit_code':r.returncode,'images':images,'passed':r.returncode==0 and all(pathlib.Path(i).exists() for i in images)})
  print(json.dumps(runs[-1]),flush=True)
  if not runs[-1]['passed']:break
report={'passed':len(runs)==1 and all(r['passed'] for r in runs),'runs':runs,'map_saved':False,'visual_review':False,'scope':'Native stationary player camera comparison; no movement through exposure transitions or release acceptance.'}
(root/'Tests/Results'/args.report).write_text(json.dumps(report,indent=2)+'\n')
