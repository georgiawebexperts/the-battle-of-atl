"""Native gunman warning, cover, dodge and fatal-hit contract."""
import pathlib,subprocess,re,json
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/gunman-core-audit.log'
with log.open('w') as f:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleGunmanAudit','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'GunmanAudit: (\{[^\n]+\})',log.read_text());r=json.loads(rows[-1]) if rows else {'passed':False,'reason':'Missing audit'}
r.update(exit_code=run.returncode,scope='Native stationary encounter fixture; warnings/cover/locked-aim dodge/fatal hit. Natural spawn rarity, incoming-fire direction, animations and rendering not accepted.');r['passed']=r['passed'] and run.returncode==0
(root/'Tests/Results/2026-09-12-gunman-core.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
