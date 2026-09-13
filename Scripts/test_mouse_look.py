"""Check engine mouse-axis input after dismount, including body/camera alignment."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import uuid

parser = argparse.ArgumentParser()
parser.add_argument('--packaged', action='store_true')
parser.add_argument('--report', required=True)
args = parser.parse_args()
assert Path(args.report).name == args.report
root = Path(__file__).resolve().parents[1]
entry = [str(root / 'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground')] if args.packaged else [
    '/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',
    str(root / 'AuraPlayground.uproject'), '-game', '-RCWebControlDisable',
]
log = root / 'work' / f'mouse-look-{uuid.uuid4().hex}.log'
with log.open('w') as stream:
    run = subprocess.run([*entry,
        '/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',
        '-nullrhi', '-BattleSkipTutorial', '-BattleFootAudit', '-BattleMouseLookAudit',
        '-unattended', '-nosound', '-stdout',
    ], stdout=stream, stderr=subprocess.STDOUT, timeout=90)
matches = re.findall(r'BattleMouseLookAudit: (\{[^\n]+\})', log.read_text())
result = json.loads(matches[-1]) if matches else {'passed': False, 'missing_report': True}
result.update(exit_code=run.returncode, packaged=args.packaged, log=str(log))
result['passed'] = bool(result.get('passed') and run.returncode == 0)
result['scope'] = 'Injected engine mouse-axis events after dismount; control, camera and body rotation. Does not verify OS pointer capture or subjective sensitivity.'
(root / 'Tests/Results' / args.report).write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result))
raise SystemExit(0 if result['passed'] else 1)
