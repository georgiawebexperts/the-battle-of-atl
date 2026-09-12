"""Explain unreachable native samples using retained OSM geometry; never add shortcuts."""
from pathlib import Path
import collections
import json
import re
from pyproj import Transformer
from shapely.geometry import LineString, Polygon

root = Path(__file__).resolve().parents[1]
terrain = root / 'SourceAssets/Terrain'
meta = json.loads((terrain / 'terrain-georeference.json').read_text())
transform = Transformer.from_crs('EPSG:4326', meta['crs'], always_xy=True)
boundary = json.loads((root / 'References/piedmont-boundary-osm.json').read_text())['elements'][0]
park = Polygon([transform.transform(p['lon'], p['lat']) for p in boundary['geometry']]).buffer(8)
ways = {w['id']: w for w in json.loads((root / 'References/piedmont-osm.json').read_text())['elements'] if w['type'] == 'way'}
network = json.loads((terrain / 'park-path-network.json').read_text())
included = {p['osm_id'] for p in network['paths']}
nav = json.loads((root / 'Tests/Results/2026-09-12-krog-world-navigation.json').read_text())
failures = collections.defaultdict(list)
for sample in nav['after']['samples']:
    if not sample['reachable']:
        match = re.search(r'OSM path (\d+)', sample['label'])
        if match:
            failures[int(match[1])].append(sample)
rows = []
for osm_id, samples in sorted(failures.items()):
    way = ways[osm_id]
    neighbors = []
    for other in ways.values():
        shared = set(way.get('nodes', [])) & set(other.get('nodes', []))
        if other['id'] == osm_id or not shared or not other.get('tags', {}).get('highway'):
            continue
        coords = [transform.transform(p['lon'], p['lat']) for p in other.get('geometry', [])]
        clipped_length = LineString(coords).intersection(park).length if len(coords) > 1 else 0
        neighbors.append({'osm_id': other['id'], 'tags': other.get('tags', {}),
                          'present_in_prepared_network': other['id'] in included,
                          'length_inside_preparation_boundary_m': clipped_length})
    rows.append({'osm_id': osm_id, 'tags': way.get('tags', {}), 'unreachable_samples': len(samples),
                 'all_projected_to_navmesh': all(s['projected'] for s in samples),
                 'shared_node_neighbors': neighbors,
                 'interpretation': 'No shared-node neighbor in retained source' if not neighbors else
                 'At least one mapped connection omitted from prepared network' if any(not n['present_in_prepared_network'] for n in neighbors) else
                 'Mapped connections retained; inspect native geometry and grades'})
report = {'scope': 'Source topology diagnosis only. Does not prove paths should be connected or accept unreachable areas.',
          'unreachable_sample_count': sum(len(v) for v in failures.values()), 'paths': rows}
output = root / 'Tests/Results/2026-09-12-disconnected-park-path-diagnosis.json'
output.write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps({'paths': len(rows), 'samples': report['unreachable_sample_count'],
                  'interpretations': dict(collections.Counter(r['interpretation'] for r in rows))}))
