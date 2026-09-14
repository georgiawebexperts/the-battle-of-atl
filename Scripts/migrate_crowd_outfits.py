"""Run in external sample: copy missing hard dependencies, never overwrite game assets."""
import unreal,json,hashlib,shutil
from pathlib import Path
source=Path(unreal.Paths.project_content_dir()).resolve();target=Path('/Volumes/Adam Assets/Unreal/Projects/AuraPlayground/Content')
assert source!=target
roots=['/Game/CitySampleCrowd/Character/'+p for p in ['Male/NormalWeight/Meshes/m_tal_nrw_buttonOpen','Male/NormalWeight/Meshes/m_tal_nrw_crewneck_blazer','Female/NormalWeight/Meshes/f_tal_nrw_buttonOpen','Female/NormalWeight/Meshes/f_tal_nrw_scoopneck_croppedJacket']]
registry=unreal.AssetRegistryHelpers.get_asset_registry();registry.search_all_assets(synchronous_search=True);options=unreal.AssetRegistryDependencyOptions(include_soft_package_references=False)
queue=list(roots);seen=set();files=[];preserved=[]
while queue:
 package=queue.pop()
 if package in seen or not package.startswith('/Game/'):continue
 seen.add(package);rel=Path(package.removeprefix('/Game/'))
 if (target/rel.with_suffix('.uasset')).exists():preserved.append(package);continue
 assert (source/rel.with_suffix('.uasset')).exists(),package
 queue.extend(str(x) for x in registry.get_dependencies(package,options))
 for suffix in ['.uasset','.uexp','.ubulk']:
  src=(source/rel).with_suffix(suffix);dst=(target/rel).with_suffix(suffix)
  if src.exists():assert not dst.exists();files.append((src,dst))
rows=[]
for src,dst in files:
 dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dst);sha=hashlib.sha256(src.read_bytes()).hexdigest();assert hashlib.sha256(dst.read_bytes()).hexdigest()==sha
 rows.append({'path':str(dst.relative_to(target)),'bytes':dst.stat().st_size,'sha256':sha})
(target.parent/'Tests/Results/2026-09-14-crowd-outfit-migration.json').write_text(json.dumps({'roots':roots,'copied':rows,'preserved_existing':preserved,'scope':'Missing hard dependencies copied. Existing project assets preserved; load/appearance compatibility not yet verified.'},indent=2)+'\n')
