"""Create georeferenced, measured USGS terrain inputs; no procedural invented elevation."""
from pathlib import Path
import json, numpy as np, rasterio
from rasterio.warp import transform_bounds,reproject,Resampling
from rasterio.windows import from_bounds
from rasterio.transform import from_origin
from pyproj import Transformer
from PIL import Image
P=Path(__file__).resolve().parents[1];out=P/'SourceAssets/Terrain';out.mkdir(parents=True,exist_ok=True)
products=json.loads((P/'References/usgs-dem-products.json').read_text())['items']
# Fixed georeference retained from the first OSM reference layer.
t=Transformer.from_crs('EPSG:4326','EPSG:32616',always_xy=True)
ox,oy=t.transform(-84.3725,33.785)
# 12 x 24 Landscape components, each 126 quads: 2 m real spacing, 1:3 scale.
nx,ny=1513,3025;spacing=2.0
west=round((ox-1100)/spacing)*spacing;south=round((oy-4000)/spacing)*spacing
north=south+(ny-1)*spacing;east=west+(nx-1)*spacing
transform=from_origin(west-spacing/2,north+spacing/2,spacing,spacing)
dest=np.full((ny,nx),-9999,dtype='float32');sources=[]
with rasterio.Env(GDAL_DISABLE_READDIR_ON_OPEN='EMPTY_DIR',CPL_VSIL_CURL_ALLOWED_EXTENSIONS='.tif',GDAL_HTTP_MAX_RETRY=3):
 for item in products:
  url=item['downloadURL'];print('Reading measured elevation:',item['title'],flush=True)
  with rasterio.open(url) as src:
   bounds=transform_bounds('EPSG:32616',src.crs,west-6,south-6,east+6,north+6,densify_pts=21)
   win=from_bounds(*bounds,src.transform).round_offsets().round_lengths()
   win=win.intersection(rasterio.windows.Window(0,0,src.width,src.height))
   data=src.read(1,window=win)
   temp=np.full_like(dest,-9999)
   reproject(source=data,destination=temp,src_transform=src.window_transform(win),src_crs=src.crs,src_nodata=src.nodata,dst_transform=transform,dst_crs='EPSG:32616',dst_nodata=-9999,resampling=Resampling.bilinear)
   mask=temp!=-9999;dest[mask]=temp[mask]
   sources.append({'title':item['title'],'url':url,'crs':str(src.crs),'window':[win.col_off,win.row_off,win.width,win.height]})
missing=int(np.sum(dest==-9999))
if missing:raise RuntimeError(f'Measured elevation missing at {missing} samples; refusing to invent terrain')
np.save(out/'atlanta-elevation-north-up.npy',dest)
# Unreal raw rows progress toward +Y (north), opposite a north-up GIS raster.
base=280.0;raw=np.round(32768+(np.flipud(dest)-base)*128).clip(0,65535).astype('<u2')
raw.tofile(out/'atlanta-height.r16');Image.fromarray(raw).save(out/'atlanta-height.png')
meta={'source':'USGS 3DEP 1 m bare-earth DEM, GA Statewide 2018 B18 DRRA','source_resolution_m':1,'sample_spacing_real_m':spacing,'scale':1/3,'crs':'EPSG:32616','origin_lon_lat':[-84.3725,33.785],'origin_utm':[ox,oy],'bounds_utm':[west,south,east,north],'size':[nx,ny],'base_elevation_m':base,'elevation_range_m':[float(dest.min()),float(dest.max())],'unreal_location_cm':[(west-ox)*100/3,(south-oy)*100/3,0],'unreal_scale':[spacing*100/3,spacing*100/3,100/3],'raw_rows':'south to north','sources':sources,'missing_samples':missing}
(out/'terrain-georeference.json').write_text(json.dumps(meta,indent=2))
print(json.dumps({k:meta[k] for k in ['size','elevation_range_m','missing_samples','unreal_location_cm']}),flush=True)
