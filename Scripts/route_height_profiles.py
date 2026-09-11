"""Authored bridge elevations over retained bare-earth terrain (source ENU cm)."""
import json
from pathlib import Path
from shapely.geometry import LineString,Point

class RouteHeightProfiles:
    def __init__(self,path):
        self.data=json.loads(Path(path).read_text())
        self.line=LineString(self.data['centerline_xy_cm'])
        self.profiles=self.data['profiles']
        for profile in self.profiles:
            from shapely.ops import substring
            profile['_bounds']=substring(self.line,profile['blend_start_cm'],profile['blend_end_cm']).buffer(profile.get('bounds_padding_cm',400)).bounds

    def height(self,x,y,ground):
        for p in self.profiles:
            a,b,c,d=p['_bounds']
            if not (a<=x<=c and b<=y<=d):continue
            s=self.line.project(Point(x,y))
            if not p['blend_start_cm']<s<p['blend_end_cm']:continue
            fraction=(s-p['blend_start_cm'])/(p['blend_end_cm']-p['blend_start_cm'])
            deck=p['start_z_cm']+(p['end_z_cm']-p['start_z_cm'])*fraction
            weight=min(1,(s-p['blend_start_cm'])/(p['bridge_start_cm']-p['blend_start_cm']),(p['blend_end_cm']-s)/(p['blend_end_cm']-p['bridge_end_cm']))
            weight=weight*weight*(3-2*weight)
            return ground+(deck-ground)*weight
        return ground
