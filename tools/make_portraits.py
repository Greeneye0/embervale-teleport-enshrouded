# Builds the prank host's portraits (assets/host-*.png) from the expression edits in
# assets/expressions/: background removed (rembg, CPU), only her own outline kept, cropped to head
# and bust at the same place in every picture, a soft fade where her arm leaves the left edge.
# The edits are images of the banner region (230, 0)-(760, 520), at any size.
# Usage: python tools/make_portraits.py [prefix]   (prefix: grok-pose or chatgpt-pose)
import sys
import numpy as np
import cv2
from PIL import Image
from rembg import remove, new_session

ROOT = 'assets/'
REGION = (530, 520)              # banner region size the edits show
CROP = (60, 40, 415, 345)        # head and bust inside that region, wide enough for raised hands
HEIGHT = 380                     # output height; small, the game's picture budget is per session
prefix = sys.argv[1] if len(sys.argv) > 1 else 'grok-pose'
session = new_session('isnet-general-use', providers=['CPUExecutionProvider'])

def portrait(path, out):
    im = Image.open(path).convert('RGB')
    sx, sy = im.width / REGION[0], im.height / REGION[1]
    cut = np.array(remove(im, session=session).convert('RGBA'))
    mask = (cut[:, :, 3] > 40).astype(np.uint8)
    n, labels, stats, _ = cv2.connectedComponentsWithStats(mask, 8)
    biggest = 1 + int(np.argmax(stats[1:, cv2.CC_STAT_AREA]))
    keep = cv2.dilate((labels == biggest).astype(np.uint8), np.ones((5, 5), np.uint8)) > 0
    cut[:, :, 3] = np.where(keep, cut[:, :, 3], 0)
    box = (round(CROP[0] * sx), round(CROP[1] * sy), round(CROP[2] * sx), round(CROP[3] * sy))
    face = Image.fromarray(cut).crop(box)
    face = face.resize((round(face.width * HEIGHT / face.height), HEIGHT), Image.LANCZOS)
    a = np.array(face).astype(np.float32)
    a[:, :, 3] *= np.clip(np.arange(a.shape[1]) / (40 * HEIGHT / 305), 0, 1)[None, :]
    Image.fromarray(a.astype(np.uint8)).save(ROOT + out)
    print(out, face.size)

portrait(ROOT + 'expressions/source.png', 'host-portrait.png')
for mood in ('worried', 'sheepish', 'wink', 'alarmed', 'smug', 'puzzled'):
    portrait(ROOT + f'expressions/{prefix}-{mood}.jpg', f'host-{mood}.png')
