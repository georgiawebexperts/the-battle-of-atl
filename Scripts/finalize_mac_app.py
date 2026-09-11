"""Brand and locally sign the archived Mac playtest, then install a desktop link."""
from pathlib import Path
import plistlib
import shutil
import subprocess
import re

root=Path(__file__).resolve().parents[1]
archive=Path('/Volumes/Adam Assets/Unreal/Builds/BattleForTheA')
version=re.search(r'^ProjectVersion=(.+)$',(root/'Config/DefaultGame.ini').read_text(),re.M).group(1).split('-')[0]

source=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
if not (source/'Contents/UE/AuraPlayground/Content/Paks/AuraPlayground-Mac.utoc').is_file():
    raise SystemExit('Staged app lacks cooked game content.')
target=archive/'Mac/BattleForTheATL.app'
if target.exists():
    raise SystemExit(f'Refusing to replace existing {target}; archive it before finalizing another build.')
shutil.copytree(source,target,symlinks=True)
plist_path=target/'Contents/Info.plist'
with plist_path.open('rb') as f:
    info=plistlib.load(f)
info['CFBundleDisplayName']='Battle for the ATL'
info['CFBundleName']='BattleForTheATL'
info['CFBundleIconFile']='BattleForTheA.icns'
info['CFBundleShortVersionString']=version
info['CFBundleVersion']=version
with plist_path.open('wb') as f:
    plistlib.dump(info,f)
shutil.copy2(root/'SourceAssets/UI/BattleForTheA.icns',target/'Contents/Resources/BattleForTheA.icns')
subprocess.run(['codesign','--force','--deep','--sign','-',str(target)],check=True)
subprocess.run(['codesign','--verify','--deep','--strict',str(target)],check=True)
desktop=Path('/Users/elliottinspace/Desktop/Battle for the ATL.app')
if desktop.is_symlink():
    desktop.unlink()
elif desktop.exists():
    raise SystemExit(f'Desktop item already exists: {desktop}')
desktop.symlink_to(target)
legacy=desktop.with_name('Battle for the A.app')
if legacy.is_symlink() and legacy.readlink()==archive/'Mac/BattleForTheA.app':
    legacy.unlink()
print(target)
