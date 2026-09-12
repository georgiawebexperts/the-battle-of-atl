"""Original deterministic small tiling textures; no third-party images."""
from pathlib import Path
import numpy as np
from PIL import Image
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/TwelfthMarket/Stall';rng=np.random.default_rng(1226);size=512;y,x=np.mgrid[:size,:size];noise=rng.normal(0,1,(size,size))
for name,base in {'Canvas':[216,208,185],'Cloth':[58,91,68],'Wood':[133,99,62]}.items():
 if name=='Wood':
  pattern=8*np.sin(y*.53+2*np.sin(x*.014))+4*np.sin(y*1.8+.5*np.sin(x*.03))+noise*2
 else:pattern=2*np.cos(x*np.pi)+2*np.cos(y*np.pi)+noise*.7+1.5*np.sin(x*2*np.pi/512)*np.sin(y*2*np.pi/512)
 rgb=np.clip(np.array(base)[None,None,:]+pattern[:,:,None],0,255).astype(np.uint8);Image.fromarray(rgb).save(out/f'T_Market_{name}.png')
