"""Regenerate the crossing-to-tunnel fixture from retained mapped route points."""
import json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
paths=json.loads((root/'SourceAssets/Terrain/krog-route-network.json').read_text())['paths']
a=next(p for p in paths if p['osm_id']==1353860787 and p['route_part']==3)
b=next(p for p in paths if p['osm_id']==722838795 and p['route_part']==4)
assert a['points_cm'][-1]==b['points_cm'][0]
points=a['points_cm'][4:]+b['points_cm'][1:18]
(root/'Source/AuraPlayground/BattleKrogRiderRoute.h').write_text('// 2026-09-12 [codex-maclaptop]; generated from retained krog-route-network.json.\n#pragma once\n#include "CoreMinimal.h"\nnamespace BattleKrogRiderRoute { inline const FVector Points[] = {\n'+''.join(f'FVector({x:.5f},{-y:.5f},{z:.5f}),\n' for x,y,z in points)+'}; }\n')
