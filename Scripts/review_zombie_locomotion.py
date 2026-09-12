"""Capture installed farmer/punk walking, running and idle poses."""
import argparse,pathlib,plistlib,subprocess,json,re,uuid
p=argparse.ArgumentParser();p.add_argument('--punk',action='store_true');p.add_argument('--editor',action='store_true');p.add_argument('--sharp',action='store_true');p.add_argument('--no-motion-blur',action='store_true');a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1];app=pathlib.Path('/Volumes/Adam Assets/Unreal/Builds/BattleForTheA/Mac/TheBattleOfATL.app')
bundle=plistlib.loads((app/'Contents/Info.plist').read_bytes())['CFBundleIdentifier'];style=('punk' if a.punk else 'vendor')+('-sharp' if a.sharp else '-no-blur' if a.no_motion_blur else '')
folder=pathlib.Path.home()/'Library/Containers'/bundle/'Data/Documents/ZombieMotionReview'/uuid.uuid4().hex;folder.mkdir(parents=True)
log=root/f'work/{style}-motion-review.log'
with log.open('w') as f:
 r=subprocess.run([*(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'-game'] if a.editor else [str(app/'Contents/MacOS/AuraPlayground')]),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',*(['-ExecCmds=r.MotionBlurQuality 0,r.AntiAliasingMethod 1'] if a.sharp else ['-ExecCmds=r.MotionBlurQuality 0'] if a.no_motion_blur else []),'-BattleSkipTutorial','-BattleLocomotionReview','-BattleZombieReview',*(['-BattlePunkReview'] if a.punk else []),f'-BattleHUDReviewDir={folder}','-RenderOffscreen','-ResX=1280','-ResY=720','-ForceRes','-NoTextureStreaming','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
lines=[x for x in log.read_text().splitlines() if 'LocoReview:' in x or 'ZombiePhysicsReview:' in x]
out={'style':style,'app':'editor' if a.editor else str(app),'exit_code':r.returncode,'measurements':lines,'images':[str(folder/name) for name in ['walk.png','walk-step.png','run.png','idle.png']],'visual_review':'pending','scope':'Fixed walking/running/idle capture; fixture metrics include idle toe orientation, not all moving foot orientations.'}
out['passed']=r.returncode==0 and any('pass=1 ' in x for x in lines) and all(pathlib.Path(x).is_file() for x in out['images'])
(root/f"Tests/Results/2026-09-12-{'editor' if a.editor else 'build052'}-{style}-motion.json").write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out));raise SystemExit(0 if out['passed'] else 1)
