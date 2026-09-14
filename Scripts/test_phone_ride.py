"""Drive from the park start to the randomly placed phone without teleporting."""
import argparse,json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--report',required=True);args=p.parse_args();assert pathlib.Path(args.report).name==args.report
log=root/'work'/('phone-ride-'+uuid.uuid4().hex+'.log')
command=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattlePhoneRideAudit','-RCWebControlDisable','-unattended','-nosound','-ResX=1280','-ResY=720','-windowed','-stdout']
with log.open('w') as f:
 try:result=subprocess.run(command,stdout=f,stderr=subprocess.STDOUT,timeout=300);code=result.returncode
 except subprocess.TimeoutExpired:code=124
text=log.read_text();rows=re.findall(r'PhoneRideAudit: (\{[^\n]+\})',text)
r=json.loads(rows[-1]) if rows else {'passed':False,'reason':'No terminal audit report'}
r.update(exit_code=code,log=str(log),fixture=re.findall(r'PhoneRideFixture: ([^\n]+)',text),failure=re.findall(r'PhoneRideFailure: ([^\n]+)',text),scope='One randomly placed phone; ordinary W/A/D/brake inputs. Existing enemies and pedestrians removed, traffic population disabled. No teleport. A guided-driver failure is not by itself evidence of a game defect. Does not establish full combat or finish-loop acceptance.')
r['passed']=r['passed'] and code==0
(root/'Tests/Results'/args.report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
