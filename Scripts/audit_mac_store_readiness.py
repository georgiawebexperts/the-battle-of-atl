"""Static checks that keep gameplay work compatible with a Mac App Store build."""
import argparse,json,pathlib,plistlib,re,subprocess,tempfile

p=argparse.ArgumentParser();p.add_argument('--report',default='2026-09-16-mac-store-readiness.json');a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
mac=(root/'Config/Mac/MacEngine.ini').read_text()
game=(root/'Config/DefaultGame.ini').read_text()
project=json.loads((root/'AuraPlayground.uproject').read_text())
release=(root/'Distribution/MAC-APP-STORE-RELEASE.md').read_text()
info=plistlib.loads((root/'Build/Mac/Resources/Info.Template.plist').read_bytes())
entitlements=plistlib.loads((root/'Build/Mac/Resources/Sandbox.NoNet.entitlements').read_bytes())

source=[]
for pattern in ('*.cpp','*.h'):
 for path in (root/'Source').rglob(pattern):source.append(path.read_text(errors='replace'))
source_text='\n'.join(source)
absolute_runtime_paths=re.findall(r'["(]/(?:Volumes|Users)/[^"\n]*',source_text)
maps=re.findall(r'^\+MapsToCook=\(FilePath="([^"]+)"\)',game,re.M)

with tempfile.TemporaryDirectory() as temporary:
 iconset=pathlib.Path(temporary)/'Battle.iconset'
 subprocess.run(['iconutil','-c','iconset',str(root/'SourceAssets/UI/BattleForTheA.icns'),'-o',str(iconset)],check=True,capture_output=True)
 icon_names={x.name for x in iconset.iterdir()}
 expected={f'icon_{size}x{size}{suffix}.png' for size in (16,32,128,256,512) for suffix in ('','@2x')}
 icon_complete=expected<=icon_names

checks={
 'bundle_identifier':'BundleIdentifier=com.webexperts.battleofatl' in mac,
 'team_recorded':'8HAG5A4GS7' in release,
 'no_distribution_setup':'CodeSigningTeam=8HAG5A4GS7' not in mac,
 'sandbox_only':entitlements=={'com.apple.security.app-sandbox':True},
 'no_network_ats':'NSAppTransportSecurity' not in info,
 'retina':info.get('NSHighResolutionCapable') is True,
 'no_external_runtime_paths':not absolute_runtime_paths,
 'production_map_only':maps==['/Game/PiedmontRide/Maps/PiedmontWorld'],
 'non_placeholder_metadata':'Disposable Aura trial' not in project.get('Description',''),
 'icon_complete':icon_complete,
 'license_inventory':all(x in release for x in ('OpenStreetMap','USGS 3DEP','Adobe Mixamo','Quaternius','Remington Model 870','Fab library','Battle of the A-T-L')),
}
report={'passed':all(checks.values()),'checks':checks,'maps_to_cook':maps,'absolute_runtime_paths':absolute_runtime_paths,'entitlements':entitlements,'bundle_id':'com.webexperts.battleofatl','apple_team_id_recorded':'8HAG5A4GS7','scope':'Static Mac App Store guardrails. Distribution identity, certificate, provisioning, notarization, App Store Connect and upload are intentionally deferred.'}
(root/'Tests/Results'/a.report).write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
