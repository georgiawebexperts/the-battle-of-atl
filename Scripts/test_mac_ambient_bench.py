"""Exercise bench ignition lifecycle in the native park world."""
import json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/packaged-ambient-bench-audit.log'
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
if '--app' in sys.argv:app=pathlib.Path(sys.argv[sys.argv.index('--app')+1])
assert app.is_file(),app
with log.open('w') as stream:
 run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleAmbientBenchAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'AmbientBenchAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Packaged encounter lifecycle with forced 0/1 probability and accelerated eligible clock; no statistical rarity, rendered or full-route acceptance.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results/2026-09-12-build046-ambient-bench.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
