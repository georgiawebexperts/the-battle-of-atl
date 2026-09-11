"""Exercise real cooked park profiles without a display; UI appearance remains a separate test."""
from pathlib import Path
import json, subprocess, tempfile, re
root=Path(__file__).resolve().parents[1]
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
results=[]
with tempfile.TemporaryDirectory(prefix='battle-difficulty-') as scratch:
    for name,seconds,population in [('Easy',900,20),('Medium',600,50),('Hard',300,90)]:
        report=Path(scratch)/f'{name}.json'
        log=root/'work'/f'mac-difficulty-{name.lower()}.log'
        with log.open('w') as stream:
            run=subprocess.run([str(app),f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={name}?AutoStart=1',
                '-nullrhi','-unattended','-nosound','-BattleAudit',f'-BattleAuditOutput={report}','-stdout'],
                stdout=stream,stderr=subprocess.STDOUT,timeout=90)
        matches=re.findall(r'BattleAudit: (\{[^\n]+\})',log.read_text())
        data=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
        quest=re.findall(r'BattleQuestAudit: ready=(\d) radar=(\d) placement=(\d) clipping=(\d) pickup=(\d) route=(\d) blocked=(\d) onFoot=(\d)',log.read_text())
        data['quest']=dict(zip(['ready','radar','placement','clipping','pickup','route','blocked','onFoot'],[bool(int(x)) for x in quest[-1]])) if quest else {}
        data.update(exit_code=run.returncode,expected_timer=seconds,expected_crowd=population)
        data['passed']=bool(data.get('passed') and run.returncode==0 and data.get('timerSeconds')==seconds and data.get('desiredCrowd')==population)
        results.append(data)
        print(json.dumps(data),flush=True)
        (root/'Tests/Results/2026-09-11-v3-native-quest.json').write_text(json.dumps({'profiles':results,'all_passed':all(r['passed'] for r in results),'rendered_ui_verified':False},indent=2)+'\n')
if not all(r['passed'] for r in results):
    raise SystemExit(1)
