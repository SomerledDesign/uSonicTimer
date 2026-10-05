# Builds the coloured STEP + VRML models in this folder for uSonicTimer_1d (footprint-origin aligned).
# Drawn by Somerled Design (Kevin Murphy) with Mildrew (Grok Bot). Licence: CERN-OHL-S-2.0.
# Dimension sources: see README.md in this folder. Run: python build_3dmodels.py  (needs cadquery)
# Coordinates written in FOOTPRINT convention (x right, y DOWN, z up from board top);
# converted to KiCad 3D-model convention (y UP) by negating y.
import cadquery as cq, os, re
OUT = os.path.dirname(os.path.abspath(__file__))

def box(x0, x1, y0, y1, z0, z1):
    return (cq.Workplane('XY').box(x1-x0, y1-y0, z1-z0, centered=False)
            .translate((x0, -y1, z0)))

def cyl(x, y, d, z0, z1):
    return cq.Workplane('XY').circle(d/2).extrude(z1-z0).translate((x, -y, z0))

COL = dict(
    silver=(0.60, 0.61, 0.64), tin=(0.74, 0.74, 0.70), black=(0.10, 0.10, 0.10),
    darkplastic=(0.16, 0.16, 0.17), green=(0.15, 0.55, 0.25), white=(0.94, 0.94, 0.92),
    glass=(0.60, 0.66, 0.56), lcdva=(0.56, 0.63, 0.52), elast=(0.12, 0.12, 0.12),
    nickel=(0.68, 0.68, 0.66))

def fix_header(path, name):
    s = open(path).read()
    head, rest = s.split('ENDSEC;', 1)
    ts = re.search(r"'(\d{4}-\d\d-\d\dT[\d:]+)'", head).group(1)
    head = ("ISO-10303-21;\nHEADER;\nFILE_DESCRIPTION(('%s: Drawn by Somerled Design (Kevin Murphy) with Mildrew (Grok Bot)',\n  'Licence: CERN-OHL-S-2.0'),'2;1');\n"
            "FILE_NAME('%s.step','%s',('Kevin Murphy'),('Somerled Design'),\n  'Open CASCADE STEP processor 7.9 (CadQuery)','CadQuery','');\n"
            "FILE_SCHEMA(('AUTOMOTIVE_DESIGN { 1 0 10303 214 1 1 1 1 }'));\n") % (name, name, ts)
    open(path, 'w').write(head + 'ENDSEC;' + rest)

def write(name, parts):
    assy = cq.Assembly(name=name)
    for i, (pname, wp, c) in enumerate(parts):
        assy.add(wp, name=pname, color=cq.Color(*COL[c], 1.0))
    assy.export(f'{OUT}/{name}.step')
    fix_header(f'{OUT}/{name}.step', name)
    # VRML (KiCad: 1 unit = 2.54 mm)
    s = 1/2.54
    with open(f'{OUT}/{name}.wrl', 'w') as f:
        f.write('#VRML V2.0 utf8\n# %s - Drawn by Somerled Design (Kevin Murphy) with Mildrew (Grok Bot), CERN-OHL-S-2.0\n' % name)
        for pname, wp, c in parts:
            shape = wp.val() if len(wp.vals()) == 1 else cq.Compound.makeCompound(wp.vals())
            verts, tris = shape.tessellate(0.02, 0.3)
            r, g, b = COL[c]
            spec = '0.5 0.5 0.5' if c in ('silver', 'tin', 'nickel') else '0.15 0.15 0.15'
            shin = '0.6' if c in ('silver', 'tin', 'nickel') else '0.2'
            f.write('DEF %s Shape {\n appearance Appearance { material Material { diffuseColor %.3f %.3f %.3f specularColor %s shininess %s } }\n' % (pname, r, g, b, spec, shin))
            f.write(' geometry IndexedFaceSet { creaseAngle 0.5\n  coord Coordinate { point [\n')
            f.write(',\n'.join('   %.5f %.5f %.5f' % (v.x*s, v.y*s, v.z*s) for v in verts))
            f.write(' ] }\n  coordIndex [\n')
            f.write(',\n'.join('   %d,%d,%d,-1' % t for t in tris))
            f.write(' ] }\n}\n')
    print('wrote', name, len(parts), 'parts')

# ---------------- ENC1: Alps EC11E with switch, vertical, 20 mm shaft ----------------
def ec11e():
    cx, cy = 7.5, 2.5                      # shaft axis (footprint), datasheet: 7.5 from A/C/B, 7 from D/E
    hx, hy = 6.0, 11.7/2                   # body 12 x 11.7 (Drawing No.3)
    Z_MOUNT, Z_TIP = 4.5, 24.5             # mounting surface 4.5, overall 24.5 (20 shaft)
    housing = box(cx-hx+0.1, cx+hx-0.1, cy-hy+0.1, cy+hy-0.1, 0, 2.0).union(
              box(cx-hx+0.5, cx+hx-0.5, cy-hy+0.5, cy+hy-0.5, 2.0, 3.8))
    frame = (box(cx-hx, cx+hx, cy-hy, cy+hy, 2.0, Z_MOUNT)
             .cut(box(cx-hx+0.5, cx+hx-0.5, cy-hy+0.5, cy+hy-0.5, 2.0, 3.8)))
    for ty in (-3.1, 8.1):                 # mounting tabs at footprint MP pads
        frame = frame.union(box(cx-1.0, cx+1.0, ty-0.25, ty+0.25, -3.5, 2.5))
    bushing = cyl(cx, cy, 7.0, Z_MOUNT, Z_MOUNT+7.0)
    shaft = cyl(cx, cy, 6.0, Z_MOUNT+7.0, Z_TIP).faces('>Z').edges().chamfer(0.5)
    flat = box(cx+1.5, cx+4, cy-4, cy+4, Z_TIP-10.0, Z_TIP+1)   # 4.5 across flat, 10 long
    shaft = shaft.cut(flat)
    pins = None
    for (px, py, bx0, bx1) in [(0, 0, -0.15, 1.7), (0, 2.5, -0.15, 1.7), (0, 5, -0.15, 1.7),
                               (14.5, 0, 13.3, 14.65), (14.5, 5, 13.3, 14.65)]:
        p = box(px-0.15, px+0.15, py-0.4, py+0.4, -3.5, 0.5).union(
            box(bx0, bx1, py-0.4, py+0.4, 0.2, 0.5))
        pins = p if pins is None else pins.union(p)
    write('RotaryEncoder_Alps_EC11E_Switch_Vertical_H20mm', [
        ('housing', housing, 'darkplastic'), ('frame', frame, 'silver'),
        ('bushing', bushing, 'nickel'), ('shaft', shaft, 'silver'), ('pins', pins, 'tin')])

# ---------------- J1: Metaltex BR4 5.0 mm, 5 ways (BR402V + BR403V) ----------------
def br4(n=5, P=5.0):
    x0, x1 = -2.5, (n-1)*P + 2.5          # 2.5 overhang (datasheet), 5.0 x ways
    yb, yf = -5.2, 5.0                     # depth 10.2, pin 5 from wire-entry face (+y)
    H, Hs = 14.2, 9.0                      # height 14.2 above PCB; shoulder (from drawing)
    yt0, yt1 = -3.2, 3.0                   # top width ~6.2 (from drawing)
    prof = (cq.Workplane('YZ').polyline([(-yb, 0), (-yf, 0), (-yf, Hs), (-yt1, H),
                                          (-yt0, H), (-yb, Hs)]).close()
            .extrude(x1-x0).translate((x0, 0, 0)))
    body = prof
    screws = None; cage = None; pins = None
    for i in range(n):
        x = i*P
        body = body.cut(box(x-1.6, x+1.6, -1.5, yf+1, 2.0, 7.8))         # wire entry
        body = body.cut(cyl(x, 0, 4.2, Hs, H+1))                           # screw well
        sc = cyl(x, 0, 4.0, Hs, H-1.8)
        sc = sc.cut(box(x-2.1, x+2.1, -0.3, 0.3, H-2.6, H)).cut(box(x-0.3, x+0.3, -2.1, 2.1, H-2.6, H))
        cg = box(x-1.5, x+1.5, -1.5, yf-0.3, 2.0, 2.5).union(box(x-1.5, x+1.5, -1.5, yf-0.3, 7.3, 7.8))
        pn = box(x-0.4, x+0.4, -0.4, 0.4, -3.5, 2.0)
        screws = sc if screws is None else screws.union(sc)
        cage = cg if cage is None else cage.union(cg)
        pins = pn if pins is None else pins.union(pn)
    # module joint between the 2-way and 3-way blocks: 0.2 wide x 0.3 deep groove round the outside
    jx = 2*P + 2.5
    body = body.cut(box(jx-0.1, jx+0.1, yb-1, yf+1, 0, H+1).cut(
        box(jx-0.2, jx+0.2, yb+0.3, yf-0.3, -1, H-0.3)))
    write('TerminalBlock_Metaltex_BR4_1x05_P5.00mm_Horizontal', [
        ('housing', body, 'green'), ('screws', screws, 'silver'), ('clamps', cage, 'nickel'),
        ('pins', pins, 'tin')])

# ---------------- U2: bare Nokia 5110/3310 LCD in metal frame ----------------
def nokia():
    FX, FY0, FY1, FH, T = 19.715, 0.0, 34.07, 4.4, 0.3
    frame = (box(-FX, FX, FY0, FY1, 0, FH)
             .cut(box(-FX+T, FX-T, FY0+T, FY1-T, -1, FH-T))
             .cut(box(-18.115, 18.115, 6.77, 32.62, 0, FH+1)))           # window 36.23 x 25.85
    for sx in (-1, 1):
        for ty in (2.5, 22.5):
            frame = frame.union(box(sx*20.25-0.2, sx*20.25+0.2, ty-1.4, ty+1.4, -1.5, 1.0)).union(
                box(min(sx*FX, sx*20.45), max(sx*FX, sx*20.45), ty-1.4, ty+1.4, 0.6, 1.0))
    guide = box(-19.35, 19.35, 4.6, 33.7, 0, 2.1)
    for sx in (-1, 1):
        for (y0, y1) in ((11, 15), (24, 28)):                              # LED pockets (silk notches)
            guide = guide.cut(box(min(sx*17.5, sx*20), max(sx*17.5, sx*20), y0, y1, -1, 3))
    guide = guide.union(cyl(-17.25, 6, 1.8, -0.8, 0.1)).union(cyl(17.25, 32.5, 1.8, -0.8, 0.1))
    elast = box(-5.0, 5.0, 1.0, 4.0, 0, 2.1)
    gy0 = (FY0+FY1)/2 - 16.5
    glass_lo = box(-19.18, 19.18, gy0, gy0+33.0, 2.1, 2.9)
    glass_up = box(-19.18, 19.18, gy0+6.4, gy0+33.0, 2.9, 4.1)
    va = box(-17.48, 17.48, gy0+33.0-1.7-23.4, gy0+33.0-1.7, 4.1, 4.12)
    write('LCD_Nokia_5110-3310_bare_frame', [
        ('frame', frame, 'silver'), ('lightguide', guide, 'white'), ('elastomer', elast, 'elast'),
        ('glass_lower', glass_lo, 'glass'), ('glass_upper', glass_up, 'glass'), ('viewing_area', va, 'lcdva')])

ec11e(); br4(); nokia()
