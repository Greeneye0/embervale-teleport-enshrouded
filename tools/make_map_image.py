import sys, math, numpy as np, cv2
from PIL import Image, ImageDraw, ImageFilter, ImageEnhance, ImageFont
src, out = sys.argv[1], sys.argv[2]
im = cv2.cvtColor(np.array(Image.open(src).convert('RGB')), cv2.COLOR_RGB2BGR)
# --- remove the "Silver Chest" hover label: white letters + their dark drop shadow
x0, y0, x1, y1 = 936, 218, 1074, 250
roi = im[y0:y1, x0:x1]
hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
letters = ((hsv[..., 2] > 190) & (hsv[..., 1] < 60)).astype(np.uint8) * 255
mask = np.zeros(im.shape[:2], np.uint8)
mask[y0:y1, x0:x1] = cv2.dilate(letters, np.ones((5, 5), np.uint8), iterations=2)
im = cv2.inpaint(im, mask, 6, cv2.INPAINT_TELEA)
cv2.imwrite(out.replace('.jpg', '-mask.png'), mask[y0-20:y1+20, x0-20:x1+20])
img = Image.fromarray(cv2.cvtColor(im, cv2.COLOR_BGR2RGB))
# --- same crop/grade as before
img = img.crop((0, 14, 2000, 650)).resize((2400, 763), Image.LANCZOS)
img = ImageEnhance.Contrast(img).enhance(1.08); img = ImageEnhance.Color(img).enhance(1.08)
img = img.filter(ImageFilter.UnsharpMask(radius=2, percent=80, threshold=2))
W, H = img.size; S = 2  # supersample overlay
def P(x, y): return ((x) * 1.2 * S, (y - 14) * 1.2 * S)
player, target = P(462, 512), P(921, 234)
gold, hot = (240, 196, 110), (255, 226, 160)
ov = Image.new('RGBA', (W * S, H * S), (0, 0, 0, 0)); d = ImageDraw.Draw(ov)
# dashed travel arc (quadratic bezier bulging upward)
cx, cy = (player[0] + target[0]) / 2 - 60 * S, min(player[1], target[1]) - 40 * S
pts = [((1-t)**2*player[0] + 2*(1-t)*t*cx + t*t*target[0], (1-t)**2*player[1] + 2*(1-t)*t*cy + t*t*target[1]) for t in [i/400 for i in range(401)]]
pts = pts[30:372]  # leave room around the arrow and the ring
for i in range(0, len(pts) - 6, 14): d.line(pts[i:i+8], fill=gold + (235,), width=5 * S)
# arrow head near target end
(ax, ay), (bx, by) = pts[-1], pts[-12]; ang = math.atan2(ay - by, ax - bx)
head = [(ax + 16*S*math.cos(ang), ay + 16*S*math.sin(ang)), (ax + 12*S*math.cos(ang+2.5), ay + 12*S*math.sin(ang+2.5)), (ax + 12*S*math.cos(ang-2.5), ay + 12*S*math.sin(ang-2.5))]
d.polygon(head, fill=gold + (245,))
# teleport ring at target: faint track + 3/4 progress arc + soft halo
tx, ty = target; r = 30 * S
d.ellipse((tx-r-26*S, ty-r-26*S, tx+r+26*S, ty+r+26*S), outline=gold + (70,), width=2 * S)
d.ellipse((tx-r, ty-r, tx+r, ty+r), outline=(30, 24, 16, 150), width=9 * S)
d.arc((tx-r, ty-r, tx+r, ty+r), start=-90, end=180, fill=hot + (255,), width=8 * S)
d.ellipse((tx-5*S, ty-5*S, tx+5*S, ty+5*S), fill=hot + (255,))
glow = ov.filter(ImageFilter.GaussianBlur(9 * S))
ov = Image.alpha_composite(glow, ov)
# caption pill
font = ImageFont.truetype('C:/Windows/Fonts/georgiab.ttf', 30 * S)
small = ImageFont.truetype('C:/Windows/Fonts/georgia.ttf', 22 * S)
cap, sub = 'Shift + Hold Left Mouse', 'one second, and you are there'
d = ImageDraw.Draw(ov)
bw = max(d.textlength(cap, font=font), d.textlength(sub, font=small)) + 44 * S
bx0, by0 = tx + 62 * S, ty - 46 * S
d.rounded_rectangle((bx0, by0, bx0 + bw, by0 + 92 * S), radius=14 * S, fill=(22, 18, 14, 205), outline=gold + (230,), width=2 * S)
d.text((bx0 + 22*S, by0 + 12*S), cap, font=font, fill=hot + (255,))
d.text((bx0 + 22*S, by0 + 54*S), sub, font=small, fill=(230, 220, 200, 235))
# wordmark bottom-left
wm = ImageFont.truetype('C:/Windows/Fonts/georgiab.ttf', 40 * S); ws = ImageFont.truetype('C:/Windows/Fonts/georgia.ttf', 22 * S)
d.rounded_rectangle((26*S, H*S - 116*S, 26*S + d.textlength('EMBERVALE TELEPORT', font=wm) + 48*S, H*S - 24*S), radius=14*S, fill=(22, 18, 14, 190), outline=gold + (200,), width=2*S)
d.text((50*S, H*S - 108*S), 'EMBERVALE TELEPORT', font=wm, fill=hot + (255,))
d.text((52*S, H*S - 60*S), 'map based teleport', font=ws, fill=(230, 220, 200, 230))
ov = ov.resize((W, H), Image.LANCZOS)
Image.alpha_composite(img.convert('RGBA'), ov).convert('RGB').save(out, quality=93)
