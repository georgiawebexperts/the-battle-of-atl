"""Prepare a Windows playtest folder; no code signing is needed."""
from pathlib import Path
import argparse, re, shutil
import win32com.client
from PIL import Image

parser = argparse.ArgumentParser()
parser.add_argument('--build', default='115')
parser.add_argument('--archive-root', required=True)
parser.add_argument('--share-root', default=None)
args = parser.parse_args()
assert re.fullmatch(r'[0-9]{3}', args.build)

version = f'0.{int(args.build)}.0'
root = Path(__file__).resolve().parents[1]
archive = Path(args.archive_root).resolve()
share_root = Path(args.share_root).resolve() if args.share_root else archive.parent / 'Share'
out = share_root / f'The Battle of ATL Windows Playtest {args.build}'
if out.exists():
    raise SystemExit(f'Share folder already exists; inspect before replacing it: {out}')

candidates = sorted(
    (p for p in archive.rglob('AuraPlayground.exe') if p.is_file()),
    key=lambda p: (len(p.relative_to(archive).parts), p.as_posix()),
)
if not candidates:
    raise SystemExit(f'No AuraPlayground.exe found under {archive}')
source = candidates[0].parent

out.mkdir(parents=True)
shutil.copytree(source, out, dirs_exist_ok=True)

exe = out / 'AuraPlayground.exe'
if not exe.exists():
    raise SystemExit(f'Expected staged executable missing: {exe}')
renamed = out / 'The Battle of ATL.exe'
if not renamed.exists():
    exe.replace(renamed)

icon_source = root / 'SourceAssets' / 'UI' / 'BattleOfATL.png'
if not icon_source.exists():
    raise SystemExit(f'Icon source missing: {icon_source}')
icon_out = out / 'BattleOfATL.ico'
sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
image = Image.open(icon_source).convert('RGBA')
frames = []
for size in sizes:
    frame = image.resize(size, Image.Resampling.LANCZOS)
    frame.save(str(icon_out), format='ICO', sizes=[size], append_images=frames)
    frames.append(frame)

for name in ['START-HERE.txt', 'CREDITS.txt']:
    template = root / 'Distribution' / name
    if not template.exists():
        raise SystemExit(f'Distribution file missing: {template}')
    (out / name).write_text(template.read_text(encoding='utf-8', errors='replace').replace('{{BUILD}}', args.build), encoding='utf-8')

shell = win32com.client.Dispatch('WScript.Shell')
desktop = Path(shell.SpecialFolders('Desktop'))
shortcut_path = desktop / f'The Battle of ATL Windows Playtest {args.build}.lnk'
shortcut = shell.CreateShortCut(str(shortcut_path))
shortcut.TargetPath = str(renamed)
shortcut.WorkingDirectory = str(out)
shortcut.IconLocation = f'{icon_out},0'
shortcut.Description = f'The Battle of ATL Windows Playtest {args.build}'
shortcut.Save()

print(out)
