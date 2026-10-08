#!/usr/bin/env python3
"""Turn the 84x48 PBM frames from preview into PNGs scaled 6x with a Nokia 5110 look.

usage: render.py <dir with .pbm files> [scale]   (needs Pillow)
"""
import glob
import os
import sys

from PIL import Image, ImageDraw

BG = (0xB4, 0xC0, 0xA8)   # LCD background
ON = (0x24, 0x2A, 0x26)   # pixel on
GRID = (0xA8, 0xB4, 0x9C) # gap between unlit pixels
GRID_ON = (0x3C, 0x44, 0x3E) # gap between lit pixels


def load_pbm(path):
    with open(path) as f:
        tokens = f.read().split()
    assert tokens[0] == "P1"
    w, h = int(tokens[1]), int(tokens[2])
    bits = "".join(tokens[3:])
    return w, h, [[bits[y * w + x] == "1" for x in range(w)] for y in range(h)]


def render(path, scale):
    w, h, px = load_pbm(path)
    border = scale * 2
    img = Image.new("RGB", (w * scale + 2 * border, h * scale + 2 * border), BG)
    d = ImageDraw.Draw(img)
    for y in range(h):
        for x in range(w):
            x0, y0 = border + x * scale, border + y * scale
            on = px[y][x]
            d.rectangle([x0, y0, x0 + scale - 1, y0 + scale - 1], fill=GRID_ON if on else GRID)
            d.rectangle([x0, y0, x0 + scale - 2, y0 + scale - 2], fill=ON if on else BG)
    out = os.path.splitext(path)[0] + ".png"
    img.save(out)
    return out


if __name__ == "__main__":
    folder = sys.argv[1] if len(sys.argv) > 1 else "."
    scale = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    for p in sorted(glob.glob(os.path.join(folder, "*.pbm"))):
        print(render(p, scale))
