"""Run the ground-material commandlet with a whitespace-free temporary script path."""
from pathlib import Path
import subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='atl-ground-') as temp:
 script=Path(temp)/'import.py';shutil.copy2(root/'Scripts/improve_ground_material.py',script)
 with (root/'work/ground-material-import.log').open('w') as log:
  r=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor',str(root/'AuraPlayground.uproject'),'-run=pythonscript','-script='+str(script),'-unattended','-nullrhi','-nosound','-stdout'],stdout=log,stderr=subprocess.STDOUT)
 raise SystemExit(r.returncode)
