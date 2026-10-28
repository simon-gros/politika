"""AC-INF-01: original POLITIKA concept-informed procedural review mesh.
This produces actual glTF 2.0 geometry + embedded PNG color texture atlas.
Not a scanned reconstruction, animation rig, or production-ready character.
"""
from __future__ import annotations
import os, math, json, zipfile, hashlib
from pathlib import Path
import numpy as np
import trimesh
from PIL import Image, ImageDraw, ImageFilter
from trimesh.visual.texture import TextureVisuals
from trimesh.visual.material import PBRMaterial

ROOT=Path(__file__).resolve().parent / 'Generated'; ROOT.mkdir(parents=True, exist_ok=True)
REFERENCE=Path(__file__).resolve().parents[4] / 'Factions/TheAtlanticConcord/Units/AC-INF-01.png'
rng=np.random.default_rng(1310)
# Material palette intentionally derived from monochrome/weakly colored sketch,
# with subdued Cold-War NATO-inspired gray-green rather than saturated camo.
COLORS={
 'field_cloth':(111,115,99),'fold_shadow':(82,88,77),'fold_highlight':(128,129,111),
 'helmet':(102,108,94),'helmet_edge':(65,70,63),'webbing':(101,102,83),
 'webbing_light':(136,132,107),'pack':(92,98,79),'pack_shadow':(75,83,69),
 'leather':(66,64,55),'boot':(52,54,49),'glove':(56,59,52),
 'metal':(46,49,47),'steel_highlight':(88,90,86),'stock':(99,88,70),
 'skin':(174,152,131),'skin_shadow':(136,113,100),'goggle_glass':(75,89,89),
 'patch':(74,83,71),'patch_line':(203,204,188)
}
assert len(COLORS)==20
# 5 columns by 4 rows with actual cloth-grain maps and subtle wear.
TILE=256; COLS=5; ROWS=4; W=COLS*TILE;H=ROWS*TILE
atlas=Image.new('RGB',(W,H)); positions={}
for i,(key,color) in enumerate(COLORS.items()):
 col=i%COLS; row=i//COLS; positions[key]=(col,row)
 yy,xx=np.mgrid[:TILE,:TILE]
 noise=rng.normal(0,4.2,(TILE,TILE))+rng.normal(0,2.0,(TILE,1))
 waves=np.sin(xx*.16+yy*.07)*1.8+np.sin(yy*.27)*1.1
 if key in ('metal','steel_highlight','goggle_glass'): noise*=.42; waves*=.6
 if key in ('skin','skin_shadow'): noise*=.55
 tile=np.clip(np.asarray(color)[None,None,:]+(noise+waves)[:,:,None],0,255).astype('uint8')
 atlas.paste(Image.fromarray(tile),(col*TILE,row*TILE))
atlas.save(ROOT/'AC-INF-01_color_atlas.png',optimize=True)
MATERIAL=PBRMaterial(name='AC_INF_01_original_color_atlas',baseColorTexture=atlas,metallicFactor=0.,roughnessFactor=.92,doubleSided=True)
meshes=[]; partlist=[]; active_frame=None

def add(name,geom,mat):
 assert name not in {x['name'] for x in partlist},name
 if active_frame is not None: geom.apply_transform(active_frame)
 geom.remove_unreferenced_vertices()
 v=np.asarray(geom.vertices); span=np.ptp(v,axis=0)
 # UV inside one coherent atlas material region, padded to prevent sampling neighbors
 center=np.min(v,axis=0)
 u=np.clip((v[:,0]-center[0])/(span[0] if span[0]>.001 else .1),0,1)
 vv=np.clip((v[:,2]-center[2])/(span[2] if span[2]>.001 else .1),0,1)
 col,row=positions[mat]; pad=0.065
 uv=np.column_stack(((col+pad+(1-2*pad)*u)/COLS,1-(row+pad+(1-2*pad)*vv)/ROWS))
 geom.visual=TextureVisuals(uv=uv,material=MATERIAL)
 meshes.append(geom)
 partlist.append({'name':name,'color_id':mat,'vertices':len(geom.vertices),'triangles':len(geom.faces)})
 return geom

def box(name,c,dim,mat):
 m=trimesh.creation.box(extents=dim);m.apply_translation(c);return add(name,m,mat)

def ellipsoid(name,c,scale,mat,sub=2):
 m=trimesh.creation.icosphere(subdivisions=sub);m.apply_scale(scale);m.apply_translation(c);return add(name,m,mat)

def tube(name,start,end,r,mat,radial=12,r_end=None):
 a=np.asarray(start,dtype=float);b=np.asarray(end,dtype=float);delta=b-a;norm=np.linalg.norm(delta)
 if norm<.00001: return None
 if r_end is None or abs(r_end-r)<1e-5:
  m=trimesh.creation.cylinder(radius=r,height=norm,sections=radial)
 else:
  # coarse cone frustum with caps
  theta=np.arange(radial)*2*np.pi/radial
  coords=np.vstack([np.column_stack((r*np.cos(theta),r*np.sin(theta),np.full(radial,-norm/2))),np.column_stack((r_end*np.cos(theta),r_end*np.sin(theta),np.full(radial,norm/2)))])
  faces=[]
  for k in range(radial):
   j=(k+1)%radial;faces.extend([[k,j,radial+k],[j,radial+j,radial+k]])
  faces.extend([[0,k+1,k] for k in range(1,radial-1)])
  faces.extend([[radial,radial+k,radial+k+1] for k in range(1,radial-1)])
  m=trimesh.Trimesh(vertices=coords,faces=faces,process=False)
 m.apply_transform(trimesh.geometry.align_vectors([0,0,1],delta/norm))
 m.apply_translation((a+b)/2)
 return add(name,m,mat)

def beam(name,a,b,width,depth,mat):
 a=np.asarray(a,dtype=float);b=np.asarray(b,dtype=float); d=b-a;ln=np.linalg.norm(d)
 m=trimesh.creation.box(extents=[width,depth,ln]);m.apply_transform(trimesh.geometry.align_vectors([0,0,1],d/ln));m.apply_translation((a+b)/2);return add(name,m,mat)

def block_shape(name,levels,mat,segments=16):
 # horizontal varying ellipses from bottom to top, caps included
 V=[];F=[]
 for z,x,y,rx,ry in levels:
  for j in range(segments):
   a=math.tau*j/segments;V.append([x+rx*math.cos(a),y+ry*math.sin(a),z])
 for i in range(len(levels)-1):
  for k in range(segments):
   n=(k+1)%segments; F.extend([[i*segments+k,i*segments+n,(i+1)*segments+n],[i*segments+k,(i+1)*segments+n,(i+1)*segments+k]])
 for k in range(1,segments-1):F.append([0,k,k+1]);F.append([(len(levels)-1)*segments,(len(levels)-1)*segments+k+1,(len(levels)-1)*segments+k])
 return add(name,trimesh.Trimesh(vertices=V,faces=F,process=False),mat)

def prism_xz(name,outline,y,thickness,mat):
 # closed extruded contour, left side then right side
 verts=[[x,y-thickness/2,z] for x,z in outline]+[[x,y+thickness/2,z] for x,z in outline]
 n=len(outline);faces=[]
 for i in range(1,n-1):faces.append([0,i+1,i]);faces.append([n,n+i,n+i+1])
 for j in range(n):k=(j+1)%n;faces.extend([[j,k,n+k],[j,n+k,n+j]])
 return add(name,trimesh.Trimesh(vertices=verts,faces=faces,process=False),mat)

def disc(name,center,normal,radius,mat,sections=12):
 a=np.asarray(center);normal=np.asarray(normal)/np.linalg.norm(normal)
 m=trimesh.creation.cylinder(radius=radius,height=.005,sections=sections)
 m.apply_transform(trimesh.geometry.align_vectors([0,0,1],normal));m.apply_translation(a);return add(name,m,mat)

# Standing height, including goggles: ~1.91m. Foot ground is z=0.
# Two readable asymmetric legs and recognizable 1980s cargo trousers.
for side,x,hip,ankle,knee,yfoot in [
 ('L',-.16,(-.145,.045,1.01),(-.235,-.16,.16),(-.19,-.095,.52),-.22),
 ('R',.16,(.155,.06,1.01),(.225,.135,.16),(.225,.08,.525),.10)]:
 x0,y0,z0=ankle; kx,ky,kz=knee;hx,hy,hz=hip
 block_shape(f'{side}_trouser_thigh',[(.52,kx,ky,.101,.104),(.60,kx*.98,ky,.119,.105),(.71,(kx+hx)*.5,(ky+hy)*.5,.119,.114),(.82,hx,hy,.112,.113),(.98,hx,hy,.108,.101)],'field_cloth')
 # pants fold panels around crotch/thigh; natural shape at knee and calf
 ellipsoid(f'{side}_knee_cloth',(kx,ky,.52),(.10,.10,.087),'field_cloth',sub=2)
 block_shape(f'{side}_trouser_shin',[(.16,x0,y0,.081,.086),(.23,x0,y0,.09,.094),(.37,(x0+kx)/2,(y0+ky)/2,.102,.103),(.47,kx,ky,.098,.105),(.51,kx,ky,.106,.105)],'field_cloth')
 for i,z in enumerate([.275,.355,.46,.57,.68,.82]):
  xc=(x0+kx)/2 if z<.5 else (kx+hx)/2; yc=(y0+ky)/2 if z<.5 else (ky+hy)/2
  ellipsoid(f'{side}_trouser_fold_{i}',(xc-.005,yc-.058,z),(.068,.019,.008),'fold_highlight' if i%2 else 'fold_shadow',1)
 # cargo pocket hangs off outer thigh
 xx=(-.275 if side=='L' else .285)
 box(f'{side}_cargo_bellows',(xx,(ky+hy)/2,.735),(.035,.143,.175),'fold_shadow')
 box(f'{side}_cargo_flap',(xx*1.055,(ky+hy)/2,.826),(.027,.150,.031),'field_cloth')
 disc(f'{side}_cargo_snap',(xx*1.14,(ky+hy)/2,.836),(1,0,0),.012,'metal')
 # angled boot upper and detailed laced combat boot
 tube(f'{side}_boot_cuff',(x0,y0,.105),(x0,y0,.29),.079,'boot',radial=14)
 ellipsoid(f'{side}_boot_toe',(x0,yfoot-.08,.073),(.080,.140,.061),'boot',sub=2)
 box(f'{side}_sole',(x0,yfoot-.092,.033),(.162,.284,.033),'leather')
 box(f'{side}_boot_tongue',(x0,yfoot-.064,.15),(.075,.021,.18),'leather')
 for l in range(6):
  z=.115+l*.028
  for sg in [-1,1]:
   tube(f'{side}_lace_{l}_{sg}',(x0+sg*.038,yfoot-.098,z),(x0-sg*.012,yfoot-.102,z+.020),.0027,'webbing_light',6)
 for n in range(4):box(f'{side}_sole_tread_{n}',(x0,yfoot-.215+n*.075,.013),(.167,.018,.016),'boot')

block_shape('jacket_tail',[(.94,0,.046,.24,.152),(1.00,0,.035,.274,.158),(1.08,0,.01,.288,.169),(1.15,0,.02,.278,.175)],'fold_shadow',24)
block_shape('jacket_body',[(1.03,0,.024,.267,.174),(1.16,0,.023,.285,.189),(1.31,0,.025,.300,.207),(1.46,0,.028,.302,.191),(1.565,0,.036,.245,.150)],'field_cloth',28)
# two front opening flaps, detailed collar, chest and button placket
for side,s in [('L',-1),('R',1)]:
 prism_xz(f'{side}_jacket_lapel',[(s*.045,1.565),(s*.19,1.555),(s*.075,1.43),(s*.015,1.472)],-.158,.026,'fold_highlight')
 box(f'{side}_shirt_chest_pocket',(s*.162,-.202,1.40),(.15,.025,.132),'field_cloth')
 box(f'{side}_chest_pocket_flap',(s*.162,-.225,1.469),(.16,.027,.035),'fold_shadow')
 disc(f'{side}_pocket_button',(s*.16,-.241,1.456),(0,-1,0),.009,'steel_highlight')
 tube(f'{side}_jacket_vertical_seam',(s*.026,-.186,1.16),(s*.026,-.168,1.42),.004,'fold_shadow',6)
box('button_placket',(-.005,-.204,1.285),(.022,.016,.31),'fold_shadow')
for k,z in enumerate([1.18,1.245,1.315,1.38,1.45]):disc(f'jacket_button_{k}',(.005,-.220,z),(0,-1,0),.009,'steel_highlight',9)
# shoulders/arms, chest webbing straps: angled rifle-ready position
for side,s in [('L',-1),('R',1)]:
 shoulder=(s*.29,0,1.505)
 elbow=(s*(.39 if side=='L' else .38),-.17 if side=='L' else -.15,1.286 if side=='L' else 1.32)
 hand=(s*.16,-.475,1.24) if side=='L' else (s*.21,-.45,1.235)
 ellipsoid(f'{side}_shoulder_fold',shoulder,(.095,.109,.100),'field_cloth')
 tube(f'{side}_upper_sleeve',shoulder,elbow,.094,'field_cloth',radial=16,r_end=.082)
 ellipsoid(f'{side}_elbow_fatigue',elbow,(.079,.080,.071),'fold_shadow')
 tube(f'{side}_lower_sleeve',elbow,hand,.072,'field_cloth',radial=16,r_end=.060)
 for i,lerp in enumerate([.25,.5,.7]):
  p=np.array(elbow)*(1-lerp)+np.array(hand)*lerp
  ellipsoid(f'{side}_sleeve_fold_{i}',p,(.052,.039,.016),'fold_highlight' if i%2 else 'fold_shadow',1)
 tube(f'{side}_wrist_cuff',np.array(hand)+[0,.036,0],np.array(hand)+[0,-.028,0],.067,'fold_shadow',12)
 ellipsoid(f'{side}_gloved_hand',hand,(.074,.069,.045),'glove',sub=2)
 for j in range(4):
  x=hand[0]+(j-1.5)*.023
  tube(f'{side}_glove_finger_{j}',(x,hand[1]-.006,hand[2]-.013),(x,hand[1]-.072,hand[2]-.027),.011,'glove',8)
 # sleeve shield-like cloth insignia and seams
 badge_x=s*.395
 prism_xz(f'{side}_insignia_shield',[(badge_x-s*.043,1.46),(badge_x+s*.043,1.46),(badge_x+s*.038,1.392),(badge_x,1.365),(badge_x-s*.038,1.392)],-.108,.011,'patch')
 disc(f'{side}_insignia_ring',(badge_x,-.119,1.419),(0,-1,0),.022,'patch_line',12)
 for q in [-1,1]:tube(f'{side}_insignia_spoke_{q}',(badge_x+q*.017,-.124,1.398),(badge_x-q*.017,-.124,1.437),.0027,'patch_line',6)
# belt with distinct buckle; layered straps across chest and over shoulders
box('web_belt',(0,-.086,1.07),(.525,.178,.07),'webbing')
box('steel_belt_buckle',(0,-.190,1.07),(.064,.018,.05),'steel_highlight')
box('belt_center',(0,-.201,1.07),(.035,.014,.026),'metal')
for side,s in [('L',-1),('R',1)]:
 beam(f'{side}_field_harness', (s*.205,-.14,1.57), (s*.096,-.218,1.11), .052,.034,'webbing')
 box(f'{side}_harness_clasp',(s*.148,-.216,1.34),(.055,.02,.04),'steel_highlight')
 for j,z in enumerate([1.105,1.185]):
  box(f'{side}_belt_utility_{j}',(s*(.22+.05*j),-.185,z),(.103,.098,.116),'webbing_light')
  box(f'{side}_belt_flap_{j}',(s*(.22+.05*j),-.238,z+.053),(.108,.014,.024),'webbing')
# prominent front ammunition pouch array (canvas rather than hard plate armor)
for i,x in enumerate([-.173,-.058,.058,.173]):
 box(f'magazine_case_{i}',(x,-.244,1.205),(.102,.066,.147),'webbing_light')
 box(f'magazine_case_flap_{i}',(x,-.283,1.269),(.103,.021,.036),'webbing')
 box(f'magazine_case_tab_{i}',(x,-.299,1.243),(.023,.010,.040),'leather')
 disc(f'magazine_case_rivet_{i}',(x,-.310,1.223),(0,-1,0),.006,'metal',8)
# neck/head: more natural face, non-facial goggles on HELMET
ellipsoid('neck',(0,-.008,1.600),(.074,.079,.111),'skin_shadow',sub=2)
ellipsoid('face_main',(0,-.029,1.716),(.111,.097,.142),'skin',sub=3)
ellipsoid('jaw',(0,-.059,1.661),(.089,.075,.068),'skin',2)
ellipsoid('chin',(0,-.091,1.624),(.062,.042,.026),'skin_shadow',2)
for s in [-1,1]:
 ellipsoid('ear_'+str(s),(s*.109,-.020,1.717),(.022,.036,.047),'skin',2)
 ellipsoid('cheekbone_'+str(s),(s*.07,-.101,1.704),(.035,.008,.016),'skin',2) ellipsoid('brow_'+str(s),(s*.054,-.120,1.766),(.045,.008,.008),'skin_shadow',2)
 ellipsoid('eye_'+str(s),(s*.052,-.119,1.744),(.010,.004,.006),'metal',2)
 ellipsoid('ear_canal_'+str(s),(s*.123,-.044,1.725),(.007,.006,.015),'skin_shadow',1)
 ellipsoid('nose_wing_'+str(s),(s*.018,-.116,1.692),(.011,.007,.009),'skin',1)
ellipsoid('nose_bridge',(0,-.112,1.717),(.018,.020,.047),'skin',2)
ellipsoid('lip',(0,-.111,1.653),(.026,.004,.003),'skin_shadow',2)
# modeled rounded combat helmet shell, shallow flare around rim
block_shape('helmet_shell',[(1.775,0,-.006,.146,.136),(1.793,0,.004,.161,.152),(1.827,0,.008,.159,.151),(1.864,0,.012,.144,.135),(1.900,0,.011,.102,.102),(1.919,0,.010,.036,.036)],'helmet',32)
block_shape('helmet_flared_skirt',[(1.765,0,.013,.176,.165),(1.779,0,.008,.173,.162),(1.800,0,.012,.159,.147)],'helmet_edge',32)
# stitched helmet cover segments and wrap strap
for j in range(12):
 t=math.tau*j/12
 a=(.158*math.cos(t),.151*math.sin(t)+.012,1.819)
 b=(.104*math.cos(t),.101*math.sin(t)+.012,1.895)
 tube(f'helmet_stitched_seam_{j}',a,b,.0029,'fold_shadow',7)
for j in range(20):
 t=math.tau*j/20;t2=math.tau*(j+1)/20
 tube(f'helmet_olive_band_{j}',(.155*math.cos(t),.149*math.sin(t)+.006,1.816),(.155*math.cos(t2),.149*math.sin(t2)+.006,1.816),.010,'webbing',8)
# protective goggles perched high on forehead / outer helmet
for s in [-1,1]:
 x=s*.079
 ellipsoid('goggle_frame_'+str(s),(x,-.148,1.864),(.069,.026,.028),'leather',2)
 ellipsoid('goggle_lens_'+str(s),(x,-.169,1.867),(.058,.011,.021),'goggle_glass',3)
 ellipsoid('goggle_glint_'+str(s),(x-.019,-.177,1.878),(.021,.002,.002),'steel_highlight',1)
beam('goggle_center_bridge',(-.022,-.181,1.873),(.022,-.181,1.873),.013,.024,'leather')
for s in [-1,1]:
 for j in range(7):
  a=math.pi*(j/7)+math.pi/2*(1 if s>0 else -1)
  b=math.pi*((j+1)/7)+math.pi/2*(1 if s>0 else -1)
  if s>0: x1=.16*math.cos(a);x2=.16*math.cos(b)
  else: x1=-.16*math.cos(a);x2=-.16*math.cos(b)
  # side strap only: visible securing goggles at temple
  if j in [0,1,2]: beam(f'goggle_strap_{s}_{j}',(s*(.145+j*.002),-.127+j*.045,1.863),(s*(.146+(j+1)*.002),-.127+(j+1)*.045,1.863),.015,.023,'leather')
# chinstrap and buckle visible below jaw
for s in [-1,1]:
 tube(f'helmet_chinstrap_{s}',(s*.145,-.014,1.788),(s*.028,-.112,1.630),.008,'leather',8)
box('chinstrap_buckle',(.047,-.109,1.666),(.025,.009,.020),'steel_highlight')
# Oversize field pack on BACK, rolled bedroll and external cargo.
block_shape('field_pack',[(1.21,0,.235,.194,.143),(1.29,0,.252,.214,.147),(1.45,0,.252,.218,.144),(1.61,0,.25,.207,.135)],'pack',22)
box('pack_large_outer_pocket',(0,.411,1.324),(.236,.070,.183),'pack_shadow')
box('pack_upper_flap',(0,.395,1.528),(.326,.082,.079),'pack_shadow')
for s in [-1,1]:
 beam(f'pack_front_shoulder_webbing_{s}',(s*.187,.08,1.54),(s*.166,-.169,1.09),.060,.027,'webbing_light')
 beam(f'pack_lashing_{s}',(s*.115,.411,1.565),(s*.115,.448,1.27),.031,.023,'webbing_light')
 box(f'pack_side_pocket_{s}',(s*.229,.275,1.3),(.076,.145,.182),'pack_shadow')
 box(f'pack_side_flap_{s}',(s*.240,.283,1.4),(.08,.144,.042),'webbing')
 box(f'pack_clasp_{s}',(s*.115,.451,1.427),(.036,.017,.024),'steel_highlight')
tube('pack_bedroll',(-.19,.319,1.616),(.19,.319,1.616),.064,'pack_shadow',16)
for s in [-1,1]:tube(f'bedroll_belt_{s}',(s*.13,.320,1.54),(s*.13,.322,1.691),.011,'webbing_light',10)
# canteen, field tool, grenade, radio and antenna at believable 1980s positions
ellipsoid('canteen',( .269,.055,1.102),(.056,.074,.096),'pack',2)
box('canteen_cap',(.270,.056,1.193),(.038,.037,.027),'metal')
box('radio_satchel',(-.283,.143,1.22),(.105,.093,.173),'webbing')
tube('radio_antenna',(-.305,.173,1.29),(-.338,.188,1.67),.0045,'metal',8)
box('grenade_spoon',( .276,-.151,1.179),(.016,.038,.043),'metal')
ellipsoid('grenade_body',( .271,-.138,1.153),(.031,.030,.046),'metal',2)
tube('grenade_ring',(.287,-.133,1.204),(.304,-.123,1.21),.008,'steel_highlight',8)
# rifle along +X across chest, stock to picture-left, muzzle to picture-right.
# weapon is a STATIC game-art prop; no mechanism/engineering data represented.
rot=trimesh.transformations.rotation_matrix(.085,[0,1,0]) # nose drops slightly in world Z
rot[:3,3]=[.015,-.452,1.304]
active_frame=rot
# fixed buttstock triangular polygon with shouldered cheek line
prism_xz('rifle_fixed_stock',[(-.59,.013),(-.51,.070),(-.275,.045),(-.274,-.061),(-.39,-.054),(-.57,-.116),(-.60,-.109)],0,.055,'stock')
box('rifle_buttplate',(-.59,.0,-.049),(.021,.067,.148),'metal')
box('rifle_receiver',(-.169,0,.0),(.245,.091,.095),'metal')
box('receiver_upper',(-.184,0,.070),(.234,.083,.033),'steel_highlight')
box('dust_cover',(-.098,-.050,.009),(.104,.011,.034),'steel_highlight')
box('receiver_pin',(-.246,-.050,-.009),(.018,.010,.020),'steel_highlight')
box('charging_handle',(-.212,.049,.082),(.055,.033,.012),'metal')
box('magazine_well',(-.047,0,-.061),(.081,.080,.054),'metal')
# curved double-stack metallic magazine built as curved set of shell segments
for n,(x,z) in enumerate([(-.057,-.10),(-.050,-.14),(-.043,-.18),(-.023,-.22)]):
 box(f'rifle_curved_magazine_segment_{n}',(x,0,z),(.073,.066,.052),'steel_highlight' if n==0 else 'metal')
 for s in [-1,1]:box(f'rifle_magazine_rib_{n}_{s}',(x+s*.026,-.035,z),(.005,.006,.041),'steel_highlight')
# grip angled back
beam('rifle_pistol_grip',(-.226,0,-.05),(-.292,0,-.182),.051,.061,'leather')
for n in range(6):tube(f'grip_checkering_{n}',(-.260, -.034,-.094-n*.011),(-.286,-.034,-.100-n*.010),.0016,'steel_highlight',6)
# trigger guard (3-segment silhouette)
for i,(a,b) in enumerate([((-.218,-.051,-.047),(-.214,-.051,-.100)),((-.214,-.051,-.100),(-.126,-.051,-.100)),((-.126,-.051,-.100),(-.105,-.051,-.051))]):tube(f'trigger_guard_{i}',a,b,.006,'metal',8)
# handguard with long ventilated shroud and heat shield
prism_xz('rifle_fore_end',[(-.04,.042),(.413,.046),(.425,-.041),(-.04,-.043)],0,.090,'stock')
box('rifle_upper_handguard',(.20,0,.048),(.456,.094,.019),'metal')
for s in [-1,1]:
 for i in range(8):
  # black oval inset vents on handguard side, readable even at RTS zoom
  ellipsoid(f'handguard_slot_{s}_{i}',(.013+i*.054,s*.048,.006),(.016,.004,.009),'metal',1)
for i in range(4):beam(f'handguard_lower_reinforcement_{i}',(.01+i*.118,-.050,-.035),(.01+i*.118,+.050,-.035),.010,.010,'steel_highlight')
# barrel, gas block, nose, post sight and muzzle
for name,a,b,r,mat in [
 ('rifle_barrel',(.42,0,.007),(.76,0,.007),.014,'metal'),
 ('rifle_gas_tube',(.34,0,.046),(.59,0,.046),.011,'metal'),
 ('rifle_front_barrel_ring',(.41,0,.007),(.446,0,.007),.023,'steel_highlight'),
 ('rifle_muzzle_brake',(.751,0,.007),(.815,0,.007),.019,'metal')]:tube(name,a,b,r,mat,14)
box('rifle_front_sight_base',(.589,0,.053),(.028,.036,.068),'metal')
box('rifle_front_sight_tip',(.589,0,.096),(.008,.009,.022),'steel_highlight')
box('rifle_rear_sight',(-.219,0,.104),(.027,.036,.024),'metal')
for s in [-1,1]:
 box(f'muzzle_ports_{s}',(.789,s*.018,.007),(.012,.005,.008),'steel_highlight')
active_frame=None
# Model identity and review-critical source provenance.
scene=trimesh.Scene()
for rec,geo in zip(partlist,meshes):scene.add_geometry(geo,geom_name=rec['name'],node_name=rec['name'])
out=ROOT/'AC-INF-01_Professional_Rifle_Section_v2.glb'
scene.export(str(out),file_type='glb')
# accessory names and meaningful mesh separation remain in the scene graph.
check=trimesh.load(out,force='scene')
triangles=sum(len(g.faces) for g in check.geometry.values())
bounds=check.bounds.tolist()
check_height=bounds[1][2]-bounds[0][2]
assert triangles>10000 and len(check.geometry)>100 and 1.83<check_height<2.05
print('export validated',len(check.geometry),triangles,bounds,'bytes',out.stat().st_size)
# Source image also provided as its own review file, not baked onto model texture.
MANIFEST={
 'asset_id':'AC-INF-01','asset_title':'The Atlantic Concord — Professional Rifle Section',
 'revision':'v2','status':'Original, procedural, concept-guided, STATIC review prototype',
 'reference':'Factions/TheAtlanticConcord/Units/AC-INF-01.png, user-provided image',
 'units':'meters','orientation':'+Z vertical; character forward -Y in source frame (glTF handles basis conversion)',
 'ground':'boots touch z ~0','bounding_box_m':bounds,
 'mesh_objects':len(check.geometry),'triangles':triangles,'materials':'Shared 20-region PBR Base Color PNG atlas, embedded in GLB; rough, non-metallic shader',
 'texture_maps':['AC-INF-01_color_atlas.png (external review copy; also embedded)'],
 'modelling_changes':['Helmet has flared rim, cloth seams, retention band, goggles mounted on top',
 'Sculpted face with eye, nose, jaw, eyebrows and chin straps',
 '1980s field fatigue jacket (pockets/collar/buttons) and cargo trousers, less modern bulky armored silhouette',
 'Backpack with strapped bedroll, utility pouches, radio, canteen, grenade and boot laces',
 'Conventional long fixed-stock rifle with curved magazine, vented handguard, barrel and sights',
 'Asymmetric forward stride pose with hands and sleeves sculpted as separate components'],
 'not_yet_production_ready':['Not scan-based or exact reconstruction of drawing; design choices are artistic approximations',
 'No armature, bone weights, animation or weapon attachment sockets',
 'Cloth and gear topology is intersecting at seams; no bespoke retopology or physics cloth',
 'No baked normals, ambient occlusion, roughness maps, LODs or collision primitives',
 'Blender opening and Unreal Engine 5.8.3 import still require real machine validation',
 'Historical military insignia used only as simplified stylization and must be reviewed']
}
(ROOT/'AC-INF-01_manifest_v2.json').write_text(json.dumps(MANIFEST,indent=2))
(ROOT/'AC-INF-01_README_v2.md').write_text('''# AC-INF-01 — Version 2: Blender / Unreal review candidate

The **GLB** includes actual triangulated meshes, UV coordinates and a **PNG base-color atlas embedded into the GLB** (an external copy is also provided). All 3D subassemblies are distinct, named objects in the scene graph. The model is deliberately static and uses a posed rifle; it has not been rigged for animation.

### Blender review

1. File → Import → glTF 2.0 (.glb/.gltf); open `AC-INF-01_Professional_Rifle_Section_v2.glb`.
2. Switch viewport shading to **Material Preview** (not Solid) to inspect the color atlas.
3. Inspect the model from the front, side and rear. The reference drawing shows all three orientations; the rifle and body stance are approximations.
4. Review pack, vest-free field jacket, rifle profile and goggles perched on the helmet.

### POLITIKA integration notes

Provisional review source file only. Scale is in **metres**; UE uses **centimetres**. Verify dimensions and axis during import, do not blindly apply a 100× scale twice. No collision/LOD/skeletal import tests have been performed. The current file is a **static mesh only** and must undergo character-art retopology, skinning and a game-camera readability test before production use.

Source: `Factions/TheAtlanticConcord/Units/AC-INF-01.png` (user-supplied drawing). The source reference is not baked into the model's textures. Generated geometry and textures are original procedural content.
''')
with zipfile.ZipFile(ROOT/'AC-INF-01_v2_Blender_Review_Package.zip','w',zipfile.ZIP_DEFLATED) as z:
 for name in [out.name,'AC-INF-01_color_atlas.png','AC-INF-01_manifest_v2.json','AC-INF-01_README_v2.md']:
  z.write(ROOT/name,arcname=name)
print(json.dumps({'model':str(out),'zip':str(ROOT/'AC-INF-01_v2_Blender_Review_Package.zip'),'mesh_objects':len(check.geometry),'triangles':triangles,'height_m':check_height},indent=2))