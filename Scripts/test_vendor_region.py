"""Native park-only zombie variant and movement fixture."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
log=root/'work/vendor-region-audit.log'
with log.open('w') as f:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleVendorRegionAudit','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'VendorRegionAudit: (\{[^\n]+\})',log.read_text())
r=json.loads(rows[-1]) if rows else {'passed':False,'reason':'Missing audit'}
r.update(exit_code=run.returncode,log=str(log),scope='Native boundary fixture; full pursuit routes, visuals and market installation remain unaccepted.')
r['pursuit_events']=re.findall(r'VendorRegion(?:Chase|Resume): ([^\n]+)',log.read_text())
r['movement_guard_verified']='VendorRegionMovement: vendor_inside=1 punk_outside=1' in log.read_text()
r['pursuit_verified']='VendorRegionChase:' in log.read_text() and 'VendorRegionResume:' in log.read_text()
r['passed']=r['pursuit_verified'] and r['passed'] and run.returncode==0 and r['movement_guard_verified']
(root/'Tests/Results/2026-09-12-native-vendor-region.json').write_text(json.dumps(r,indent=2)+'\n')
print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
