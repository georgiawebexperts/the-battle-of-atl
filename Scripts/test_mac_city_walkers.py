"""Capture and validate the two native City Sample walker outfits."""
import json, re, subprocess, plistlib, uuid, shutil, argparse
p=argparse.ArgumentParser();p.add_argument("--bump",action="store_true");p.add_argument("--hard",action="store_true");args=p.parse_args();args.bump=args.bump or args.hard
from pathlib import Path
root=Path(__file__).resolve().parents[1]
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
reports=[]
for variant in ['male','female']:
 capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleCityReview'/uuid.uuid4().hex
 capture.mkdir(parents=True)
 out=root/('work/build044-city-knockdown-review' if args.hard else 'work/build044-city-bump-review' if args.bump else 'work/build044-city-review')/variant;out.mkdir(parents=True,exist_ok=True)
 flags=['-BattleCityFemale'] if variant=='female' else []
 if args.bump:flags.append('-BattleCityBumpReview')
 if args.hard:flags.append('-BattleCityKnockdownReview')
 with (out/'run.log').open('w') as log:
  result=subprocess.run([str(bundle/'Contents/MacOS/AuraPlayground'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-BattleSkipTutorial','-BattleLocomotionReview','-BattleCityReview',*flags,f'-BattleHUDReviewDir={capture}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=120)
 markers=re.findall(r'CityKnockdownReview: ([^\n]+)' if args.hard else r'CityBumpReview: ([^\n]+)' if args.bump else r'CityLocoReview: ([^\n]+)',(out/'run.log').read_text())
 missing=[]
 for name in (['bump-0','bump-1','bump-2','bump-3'] if args.bump else ['walk','walk-step','run','idle']):
  source=capture/f'{name}.png'
  if source.exists():shutil.copy2(source,out/source.name)
  else:missing.append(name)
 report={'variant':variant,'passed':result.returncode==0 and len(markers)==1 and markers[0].startswith('pass=1 ') and not missing,'exit_code':result.returncode,'measurements':markers,'missing_captures':missing,'visual_review':'pending'}
 reports.append(report);print(json.dumps(report),flush=True)
(root/('Tests/Results/2026-09-11-build044-city-knockdowns.json' if args.hard else 'Tests/Results/2026-09-11-build044-city-bumps.json' if args.bump else 'Tests/Results/2026-09-11-build044-city-walkers.json')).write_text(json.dumps(reports,indent=2)+'\n')
raise SystemExit(0 if all(r['passed'] for r in reports) else 1)
