"""Run in the external Epic sample project to copy complete package dependencies.
Existing game files must match byte-for-byte; this never overwrites a different asset.
"""
import unreal, pathlib, json, hashlib, shutil
source=pathlib.Path(unreal.Paths.project_content_dir()).resolve()
target=pathlib.Path('/Volumes/Adam Assets/Unreal/Projects/AuraPlayground/Content')
report=target.parent/'work/epic-tree-migration.json'
roots=['/Game/EuropeanHornbeam/Geometry/SimpleWind/SM_EuropeanHornbeam_'+name for name in ['Forest_01','Field_01','Sapling_01']]
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(synchronous_search=True)
options=unreal.AssetRegistryDependencyOptions()
visited=set();external=set();queue=list(roots)
while queue:
 package=queue.pop()
 if package in visited:continue
 if not package.startswith('/Game/'):
  external.add(package);continue
 visited.add(package)
 queue.extend(str(p) for p in r.get_dependencies(package,options))
files=[]
def digest(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for block in iter(lambda:f.read(1024*1024),b''):h.update(block)
 return h.hexdigest()
# Validate the entire copy before mutating the destination.
for package in sorted(visited):
 rel=pathlib.Path(package.removeprefix('/Game/'));base=source/rel
 found=False
 for suffix in ['.uasset','.umap','.uexp','.ubulk']:
  src=base.with_suffix(suffix)
  if not src.exists():continue
  found=True;dst=target/rel.with_suffix(suffix);sha=digest(src)
  if dst.exists() and digest(dst)!=sha:raise RuntimeError('Conflicting existing asset: '+str(dst))
  files.append({'source':str(src),'target':str(dst),'sha256':sha,'bytes':src.stat().st_size})
 if not found:raise RuntimeError('Missing dependency package: '+package)
for row in files:
 dst=pathlib.Path(row['target']);dst.parent.mkdir(parents=True,exist_ok=True)
 if not dst.exists():shutil.copy2(row['source'],dst)
 assert digest(dst)==row['sha256']
report.write_text(json.dumps({'roots':roots,'packages':sorted(visited),'external_dependencies':sorted(external),'files':files,'total_bytes':sum(f['bytes'] for f in files),'status':'copied; native render acceptance pending'},indent=2)+'\n')
print('BATTLE_TREE_MIGRATION_COMPLETE',len(files))
