"""Prepare a separate locally signed playtest; never replace the installed app."""
from pathlib import Path
import json, plistlib, shutil, subprocess
root=Path(__file__).resolve().parents[1]
out=Path('/Volumes/Adam Assets/Unreal/Builds/Share/The Battle of ATL Mac Playtest 059')
if out.exists(): raise SystemExit('Share folder already exists; inspect before replacing it.')
out.mkdir(parents=True)
source=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
app=out/'The Battle of ATL.app'
shutil.copytree(source,app,symlinks=True)
p=app/'Contents/Info.plist';data=plistlib.loads(p.read_bytes())
data.update(CFBundleDisplayName='The Battle of ATL',CFBundleName='TheBattleOfATL',CFBundleShortVersionString='0.59.0',CFBundleVersion='0.59.0',CFBundleIconFile='BattleForTheA.icns')
p.write_bytes(plistlib.dumps(data))
shutil.copy2(root/'SourceAssets/UI/BattleForTheA.icns',app/'Contents/Resources/BattleForTheA.icns')
for name in ['START-HERE.txt','CREDITS.txt']:
 shutil.copy2(root/'Distribution'/name,out/name)
 shutil.copy2(root/'Distribution'/name,app/'Contents/Resources'/name)
subprocess.run(['codesign','--force','--deep','--sign','-',str(app)],check=True)
subprocess.run(['codesign','--verify','--deep','--strict',str(app)],check=True)
print(out)
