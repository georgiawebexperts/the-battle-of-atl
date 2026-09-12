"""Bake original fictional stall lettering onto reusable sign panels."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/TwelfthMarket/Stall';names=['PEACHES & GREENS','THE GARDEN PATCH','FRESH PICKED','ROOTS & SHOOTS','SUNDAY TOMATOES']
for i,name in enumerate(names):
 image=Image.new('RGB',(1024,144),(24,46,34));d=ImageDraw.Draw(image);d.rectangle((8,8,1015,135),outline=(185,173,132),width=3)
 font=ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial Bold.ttf',72);box=d.textbbox((0,0),name,font=font)
 d.text(((1024-(box[2]-box[0]))/2,72-(box[3]+box[1])/2),name,font=font,fill=(241,233,206));image.save(out/f'T_MarketSign_{i}.png')
(out/'SM_MarketSign.obj').write_text('''# Original sign, source Y reflected by OBJ import.
v -135 -155 184
v 135 -155 184
v 135 -155 220
v -135 -155 220
vt 0 0
vt 1 0
vt 1 1
vt 0 1
f 1/1 2/2 3/3
f 1/1 3/3 4/4
''')
(out/'signs.json').write_text(json.dumps({'names':names,'texture_size':[1024,144],'source_font':'Arial Bold, macOS system font; rasterized lettering only','geometry':'SM_MarketSign.obj'},indent=2)+'\n')
