# Genera Assets/background.png (1800x1200 = 900x600 @2x) y Assets/preview.png
import numpy as np, subprocess
from PIL import Image, ImageDraw, ImageFilter, ImageFont

LOGO = "/mnt/user-data/uploads/LOGO2024.PNG"
S, W, H = 2, 1040, 600
w, h = W * S, H * S
sc = lambda v: int(round(v * S))
def font(style, size):
    p = subprocess.check_output(["fc-match", "-f", "%{file}", style]).decode()
    return ImageFont.truetype(p, int(round(size * S)))

CY, MG, VI, OR = (45, 226, 255), (255, 61, 139), (138, 92, 255), (255, 154, 61)
def lerp(a, b, t): return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))
def grad(t):
    st = [(0, OR), (0.28, MG), (0.6, VI), (1, CY)]
    t = min(max(t, 0), 1)
    for (t0, c0), (t1, c1) in zip(st, st[1:]):
        if t <= t1: return lerp(c0, c1, (t - t0) / (t1 - t0))
    return CY

# ---------- layout (puntos) : debe coincidir con PluginEditor.cpp ----------
PANELS = [  # x, y, w, h, titulo
    (24, 116, 460, 230, "GRAPHIC EQ"),
    (496, 116, 520, 230, "DYNAMICS"),
    (24, 358, 220, 220, "INPUT / OUTPUT"),
    (256, 358, 220, 220, "COLOR"),
    (488, 358, 220, 220, "LIMITER"),
    (720, 358, 296, 220, "LOUDNESS"),
]

# ---------- fondo base ----------
yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
t = (xx / w * 0.5 + yy / h * 0.5)[..., None]
base = np.array([6, 7, 12]) + (np.array([15, 17, 34]) - np.array([6, 7, 12])) * t
def glow(cx, cy, r, col, a):
    d = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2)
    return (np.clip(1 - d / r, 0, 1) ** 2 * a)[..., None] * np.array(col)
base = base + glow(w * .06, h * 1.0, w * .55, MG, .32) + glow(w * .96, 0, w * .55, CY, .26) + glow(w * .5, h * .55, w * .5, VI, .12)
vig = 1 - 0.30 * (((xx - w / 2) / (w / 2)) ** 2 * 0.6 + ((yy - h / 2) / (h / 2)) ** 2 * 0.4)
base = np.clip(base * vig[..., None], 0, 255).astype(np.uint8)
bg = Image.fromarray(base, "RGB").convert("RGBA")

# rejilla de puntos
dots = Image.new("RGBA", (w, h), (0, 0, 0, 0)); dd = ImageDraw.Draw(dots)
for gx in range(sc(12), w, sc(18)):
    for gy in range(sc(12), h, sc(18)):
        dd.ellipse([gx - 1, gy - 1, gx + 1, gy + 1], fill=(160, 180, 220, 26))
bg = Image.alpha_composite(bg, dots)

# ---------- ondas que "evolucionan": caos -> seno limpio ----------
def wave(cy, amp, phase, f, seed, chaos_pow=1.7, clampk=99):
    r = np.random.default_rng(seed)
    xs = np.arange(w); tt = xs / (w - 1)
    k = np.exp(-0.5 * (np.arange(-6, 7) / 1.8) ** 2); k /= k.sum()
    noise = np.convolve(r.normal(0, 1, w), k, "same") * 2.4
    hf = np.sin(xs * 0.35 + phase * 3) * .5 + np.sin(xs * 0.91 + phase) * .3
    chaos = (1 - tt) ** chaos_pow
    y = cy + amp * (np.sin(2 * np.pi * f * tt + phase) * (.75 + .25 * tt) + chaos * (noise * .9 + hf) * .9)
    y = np.clip(y, cy - amp * clampk, cy + amp * clampk)
    return xs, y, tt
def draw_wave(d, cy, amp, phase, f, seed, width, alpha, clampk=99):
    xs, y, tt = wave(cy, amp, phase, f, seed, clampk=clampk)
    for i in range(0, w - 3, 3):
        d.line([(xs[i], y[i]), (xs[i + 3], y[i + 3])], fill=grad(tt[i]) + (alpha,), width=width)
def ribbon(specs, blur):
    lay = Image.new("RGBA", (w, h), (0, 0, 0, 0)); d = ImageDraw.Draw(lay)
    for s in specs: draw_wave(d, *s)
    g = lay.filter(ImageFilter.GaussianBlur(blur))
    return Image.alpha_composite(Image.alpha_composite(Image.new("RGBA", (w, h), (0, 0, 0, 0)), g), lay)

big = ribbon([(h * .56, h * .20, p, f, sd, sc(2), a) for p, f, sd, a in
              [(0.0, 2.3, 1, 95), (0.7, 2.3, 2, 75), (1.4, 2.0, 3, 60), (2.1, 2.6, 4, 45)]], sc(5))
bg = Image.alpha_composite(bg, big)

# ---------- paneles de vidrio esmerilado ----------
blur = bg.filter(ImageFilter.GaussianBlur(sc(7)))
font_t = font("DejaVu Sans:bold", 10.5)
def tracked(d, xy, text, f, fill, tr):
    x, y = xy
    for ch in text:
        d.text((x, y), ch, font=f, fill=fill); x += d.textlength(ch, font=f) + tr
    return x
ov = Image.new("RGBA", (w, h), (0, 0, 0, 0)); od = ImageDraw.Draw(ov)
for (x, y, pw, ph, title) in PANELS:
    m = Image.new("L", (w, h), 0)
    ImageDraw.Draw(m).rounded_rectangle([sc(x), sc(y), sc(x + pw), sc(y + ph)], radius=sc(14), fill=255)
    bg = Image.composite(blur, bg, m)
    tint = Image.alpha_composite(bg, Image.new("RGBA", (w, h), (9, 11, 18, 172)))
    bg = Image.composite(tint, bg, m)
    od.rounded_rectangle([sc(x), sc(y), sc(x + pw), sc(y + ph)], radius=sc(14), outline=(255, 255, 255, 30), width=sc(1))
    x0, x1 = sc(x + 16), sc(x + pw - 16)
    for i in range(x0, x1, 4):
        od.line([(i, sc(y) + 1), (i + 4, sc(y) + 1)], fill=grad(1 - (i - x0) / (x1 - x0)) + (200,), width=sc(1.6))
    od.ellipse([sc(x + 16), sc(y + 15), sc(x + 22), sc(y + 21)], fill=CY + (255,))
    tracked(od, (sc(x + 30), sc(y + 11)), title, font_t, (170, 188, 214, 255), sc(2))
bg = Image.alpha_composite(bg, ov)

# ---------- barras espectrales en el borde inferior ----------
bars = Image.new("RGBA", (w, h), (0, 0, 0, 0)); bd = ImageDraw.Draw(bars)
r = np.random.default_rng(11); n = 83
for i in range(n):
    tt = i / (n - 1); chaos = (1 - tt) ** 1.4
    v = .5 + .5 * np.sin(tt * 9) * (.6 + .4 * tt)
    v = v * (1 - chaos) + r.random() * chaos
    bh = sc(2 + 9 * v); bx = sc(24 + i * 12)
    bd.rounded_rectangle([bx, sc(596) - bh, bx + sc(5), sc(596)], radius=sc(1.5), fill=grad(1 - tt) + (150,))
bg = Image.alpha_composite(bg, bars.filter(ImageFilter.GaussianBlur(sc(2))))
bg = Image.alpha_composite(bg, bars)

# ---------- divisor del header (onda) ----------
hd = ribbon([(sc(100), sc(7), p, f, sd, sc(2), a, 2.0) for p, f, sd, a in
             [(0.0, 3.0, 21, 255), (0.9, 3.0, 22, 170), (1.8, 2.6, 23, 110)]], sc(4))
bg = Image.alpha_composite(bg, hd)

# ---------- logo ----------
logo = Image.open(LOGO).convert("RGBA")
a = np.array(logo)[:, :, 3]; ys, xs = np.where(a > 10)
pad = 14
logo = logo.crop((xs.min() - pad, ys.min() - pad, xs.max() + pad, ys.max() + pad))
al = np.array(logo)[:, :, 3]
# El logo se usa tal cual: los huecos de a, g, z quedan transparentes.
import scipy.ndimage as ndi
from PIL import ImageChops
_lab, _n = ndi.label(al <= 10)
counters = ((_lab > 0) & (_lab != _lab[0, 0])).astype(np.uint8) * 255
counters = ndi.binary_dilation(counters > 0, iterations=3).astype(np.uint8) * 255
counter_img = Image.fromarray(counters, "L")
lw = sc(250); lh = int(logo.height * lw / logo.width)
logo = logo.resize((lw, lh), Image.LANCZOS)
lx, ly = sc(26), sc(50) - lh // 2
lm = logo.split()[3]
halo = Image.new("RGBA", (w, h), (0, 0, 0, 0))
cyan_l = Image.new("RGBA", logo.size, CY + (255,)); cyan_l.putalpha(lm.point(lambda v: int(v * .9)))
halo.paste(cyan_l, (lx, ly)); halo = halo.filter(ImageFilter.GaussianBlur(sc(9)))
cm = Image.new("L", (w, h), 0); cm.paste(counter_img.resize((lw, lh), Image.LANCZOS), (lx, ly))
cm = cm.filter(ImageFilter.GaussianBlur(sc(0.6)))
ha = ImageChops.multiply(halo.split()[3], ImageChops.invert(cm)); halo.putalpha(ha)
bg = Image.alpha_composite(bg, halo)
mg_l = Image.new("RGBA", (w, h), (0, 0, 0, 0)); mg_t = Image.new("RGBA", logo.size, MG + (255,)); mg_t.putalpha(lm.point(lambda v: int(v * .5)))
mg_l.paste(mg_t, (lx + sc(2), ly + sc(3))); mg_b = mg_l.filter(ImageFilter.GaussianBlur(sc(12)))
mg_b.putalpha(ImageChops.multiply(mg_b.split()[3], ImageChops.invert(cm)))
bg = Image.alpha_composite(bg, mg_b)
lay = Image.new("RGBA", (w, h), (0, 0, 0, 0)); lay.paste(logo, (lx, ly)); bg = Image.alpha_composite(bg, lay)

# ---------- titulo SOUNDEVOLVE ----------
ft = font("DejaVu Sans:bold", 27); tr = sc(6)
tx, ty = sc(296), sc(50) - int(ft.size * .52)
mask = Image.new("L", (w, h), 0); md = ImageDraw.Draw(mask)
endx = tracked(md, (tx, ty), "SOUNDEVOLVE", ft, 255, tr)
gx = Image.new("RGBA", (w, h)); gp = gx.load()
colr = Image.new("RGBA", (w, h), (255, 255, 255, 255))
gl = Image.new("RGBA", (w, h), (0, 0, 0, 0)); gd = ImageDraw.Draw(gl)
for i in range(tx, int(endx) + 1):
    gd.line([(i, 0), (i, h)], fill=lerp((255, 255, 255), CY, ((i - tx) / (endx - tx)) ** 1.5) + (255,))
tl = Image.new("RGBA", (w, h), (0, 0, 0, 0)); tl.paste(gl, (0, 0), mask)
tg = Image.new("RGBA", (w, h), CY + (255,)); tg.putalpha(mask.filter(ImageFilter.GaussianBlur(sc(6))).point(lambda v: int(v * .7)))
bg = Image.alpha_composite(bg, tg); bg = Image.alpha_composite(bg, tl)
d = ImageDraw.Draw(bg)
tracked(d, (tx + sc(2), sc(74)), "MASTERING TOOL  /  LOUDNESS STANDARDS", font("DejaVu Sans:bold", 7.5), (140, 158, 190, 255), sc(1.6))
tracked(d, (sc(760), sc(26)), "PRESET", font("DejaVu Sans:bold", 8), (140, 158, 190, 255), sc(2))

# ruido fino anti-banding
arr = np.array(bg.convert("RGB")).astype(np.float32)
arr += np.random.default_rng(3).normal(0, 1.4, arr.shape)
final = Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), "RGB")
final.save("background.png", optimize=True)
final.convert("RGB").save("/tmp/bg_check.png")

# ---------- PREVIEW (aproxima la UI final con controles) ----------
names = ["31","62","125","250","500","1k","2k","4k","8k","16k","Comp Thresh","Comp Ratio","Attack","Release","Makeup","Drive","Width %","Limiter In","Ceiling","Input","Output"]
mins = [-12]*10 + [-40,1,1,10,0,0,0,0,-6,-12,-24]
maxs = [12]*10 + [0,10,100,500,12,12,200,15,0,12,0]
vals = [.5,.5,0,-.5,-.5,0,.5,.5,1,1, -14,2,30,150,1.5,2,105,5,-.1,0,0]
dec  = [1]*10 + [1,1,1,0,1,1,0,1,1,1,1]
pv = final.convert("RGBA"); pl = Image.new("RGBA", (w, h), (0, 0, 0, 0)); pd = ImageDraw.Draw(pl)
glow_l = Image.new("RGBA", (w, h), (0, 0, 0, 0)); gd2 = ImageDraw.Draw(glow_l)
f_l, f_v, f_s = font("DejaVu Sans", 10), font("DejaVu Sans", 10.5), font("DejaVu Sans", 8)
def ctext(x, y, s_, f, fill): pd.text((sc(x), sc(y)), s_, font=f, fill=fill, anchor="mm")

# --- faders del EQ gráfico
px, py, pw, ph, _ = PANELS[0]
fy0, fh = py + 40, 160
def ypos(v): return fy0 + 8 + (fh - 16) * (1 - (v + 12) / 24)
for v in (12, 6, 0, -6, -12):
    pd.text((sc(px + 24), sc(ypos(v))), ("+" if v > 0 else "") + str(v), font=f_s, fill=(140, 158, 190, 255), anchor="rm")
for i in range(10):
    fx = px + 26 + i * 42; cx = fx + 21
    for v in (-12, -6, 6, 12): pd.line([(sc(fx), sc(ypos(v))), (sc(fx + 42), sc(ypos(v)))], fill=(255, 255, 255, 18), width=sc(1))
    pd.line([(sc(fx), sc(ypos(0))), (sc(fx + 42), sc(ypos(0)))], fill=(255, 255, 255, 52), width=sc(1))
    pd.rounded_rectangle([sc(cx - 2.5), sc(ypos(12) - 4), sc(cx + 2.5), sc(ypos(-12) + 4)], radius=sc(2.5), fill=(35, 40, 54, 255))
    yv = ypos(vals[i]); y0 = ypos(0)
    col = MG if vals[i] > 0 else CY
    if abs(yv - y0) > .1:
        box = [sc(cx - 2.5), sc(min(yv, y0)), sc(cx + 2.5), sc(max(yv, y0))]
        pd.rounded_rectangle(box, radius=sc(2), fill=col + (255,)); gd2.rounded_rectangle(box, radius=sc(2), fill=col + (255,))
    pd.rounded_rectangle([sc(cx - 9), sc(yv - 5), sc(cx + 9), sc(yv + 5)], radius=sc(3), fill=(235, 240, 250, 255))
    pd.line([(sc(cx - 6), sc(yv)), (sc(cx + 6), sc(yv))], fill=(0, 0, 0, 110), width=sc(1))
    ctext(cx, fy0 + fh + 11, names[i], f_l, (184, 196, 216, 255))

# --- perillas
groups = [(1, [10, 11, 12, 13, 14]), (2, [19, 20]), (3, [15, 16]), (4, [17, 18])]
for pi, plist in groups:
    px, py, pw, ph, _ = PANELS[pi]
    for i, p in enumerate(plist):
        cx0 = px + 10 + i * 100; cy0 = py + 34; ch = ph - 44
        ctext(cx0 + 50, cy0 + 9, names[p], f_l, (184, 196, 216, 255))
        sl_top, sl_h = cy0 + 18, ch - 18
        kc = (cx0 + 50, sl_top + (sl_h - 20) / 2); arcR = 37; lw_ = 6
        n_ = (vals[p] - mins[p]) / (maxs[p] - mins[p])
        bb = [sc(kc[0] - arcR), sc(kc[1] - arcR), sc(kc[0] + arcR), sc(kc[1] + arcR)]
        pd.ellipse([sc(kc[0] - 28), sc(kc[1] - 28), sc(kc[0] + 28), sc(kc[1] + 28)], fill=(20, 23, 33, 255), outline=(255, 255, 255, 25))
        pd.arc(bb, 126, 126 + 288, fill=(35, 40, 54, 255), width=sc(lw_))
        steps = max(2, int(n_ * 60))
        for s_ in range(steps):
            a0 = 126 + 288 * n_ * s_ / steps; a1 = 126 + 288 * n_ * (s_ + 1) / steps + 1
            col = lerp(CY, MG, (s_ / steps) * n_)
            gd2.arc(bb, a0, a1, fill=col + (255,), width=sc(lw_ + 5)); pd.arc(bb, a0, a1, fill=col + (255,), width=sc(lw_))
        ang = np.deg2rad(126 + 288 * n_)
        ex, ey = kc[0] + 22 * np.cos(ang), kc[1] + 22 * np.sin(ang); sx, sy = kc[0] + 9 * np.cos(ang), kc[1] + 9 * np.sin(ang)
        pd.line([(sc(sx), sc(sy)), (sc(ex), sc(ey))], fill=(255, 255, 255, 255), width=sc(3))
        ctext(cx0 + 50, sl_top + sl_h - 9, f"{vals[p]:.{dec[p]}f}", f_v, (232, 238, 248, 255))
gb = glow_l.filter(ImageFilter.GaussianBlur(sc(5))); gb.putalpha(gb.split()[3].point(lambda v: int(v * .45)))
pv = Image.alpha_composite(pv, gb)

# --- selector de presets
pd.rounded_rectangle([sc(760), sc(44), sc(1016), sc(76)], radius=sc(6), fill=(16, 19, 28, 255), outline=(255, 255, 255, 45))
pd.text((sc(774), sc(60)), "Streaming (-14 LUFS)", font=font("DejaVu Sans", 12), fill=(255, 255, 255, 255), anchor="lm")
pd.polygon([(sc(996), sc(57)), (sc(1006), sc(57)), (sc(1001), sc(63))], fill=CY + (255,))

# --- loudness
cap = font("DejaVu Sans:bold", 8)
for (cx_, txt, col, val) in [(800, "MOMENTARY", (255, 255, 255), "-13.8"), (936, "INTEGRATED", CY, "-14.1")]:
    tw = sum(pd.textlength(c, font=cap) + sc(2) for c in txt)
    tracked(pd, (sc(cx_) - tw / 2, sc(392) - sc(5)), txt, cap, (140, 158, 190, 255), sc(2))
    pd.text((sc(cx_), sc(440)), val, font=font("DejaVu Sans:bold", 34), fill=col + (255,), anchor="mm")
    pd.text((sc(cx_), sc(476)), "LUFS", font=font("DejaVu Sans", 10), fill=(140, 158, 190, 255), anchor="mm")
bx0, bw = 736, 264
pd.rounded_rectangle([sc(bx0), sc(500), sc(bx0 + bw), sc(510)], radius=sc(5), fill=(30, 34, 46, 255))
fx = bx0 + bw * (-13.8 + 40) / 40
for i in range(sc(bx0), sc(fx), 3): pd.line([(i, sc(501)), (i, sc(509))], fill=lerp(CY, MG, (i - sc(bx0)) / (sc(fx) - sc(bx0))) + (255,), width=3)
for lv, tx_, row in [(-23, "-23 EBU", 0), (-16, "-16 Apple", 1), (-14, "-14 Spotify", 0)]:
    xx_ = bx0 + bw * (lv + 40) / 40
    pd.line([(sc(xx_), sc(497)), (sc(xx_), sc(513))], fill=(255, 255, 255, 200), width=sc(1))
    pd.text((sc(xx_), sc(522 + 12 * row)), tx_, font=font("DejaVu Sans", 8), fill=(160, 176, 204, 255), anchor="mm")
pd.rounded_rectangle([sc(736), sc(544), sc(846), sc(570)], radius=sc(6), fill=(16, 19, 28, 255), outline=(255, 255, 255, 45))
pd.text((sc(791), sc(557)), "Reset LUFS", font=font("DejaVu Sans", 11), fill=(255, 255, 255, 255), anchor="mm")
pv = Image.alpha_composite(pv, pl)
pv.convert("RGB").save("preview.png", optimize=True)
print("ok", final.size, logo.size)
