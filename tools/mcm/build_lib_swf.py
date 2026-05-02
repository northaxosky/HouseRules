#!/usr/bin/env python3
"""Build Data/MCM/Config/HouseRules/lib.swf from a source image.

The SWF contains one bitmap exported as the AS3 class 's1', referenced by
config.json via { "type": "image", "libName": "HouseRules", "className": "s1" }.

Pure Python — no FFDec or other Flash toolchain at build time. Pillow is the
only dependency. Re-runnable; commit the resulting lib.swf alongside the rest
of Data/MCM/Config/HouseRules/.

Run:
    python tools/mcm/build_lib_swf.py
    python tools/mcm/build_lib_swf.py --max-width 1080
"""

from __future__ import annotations

import argparse
import struct
import sys
import zlib
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required. Install with: python -m pip install Pillow")


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_INPUT = Path(__file__).resolve().parent / "house.webp"
DEFAULT_OUTPUT = REPO_ROOT / "Data" / "MCM" / "Config" / "HouseRules" / "lib.swf"

SWF_VERSION = 10  # Flash CS4 era; supports SymbolClass + AS3
TWIPS = 20        # 1 px = 20 twips in SWF coordinate space

TAG_END                   = 0
TAG_SHOW_FRAME            = 1
TAG_SET_BACKGROUND_COLOR  = 9
TAG_DEFINE_BITS_LOSSLESS2 = 36
TAG_FILE_ATTRIBUTES       = 69
TAG_SYMBOL_CLASS          = 76


def encode_tag(tag_type: int, body: bytes) -> bytes:
    if len(body) < 0x3f:
        header = struct.pack("<H", (tag_type << 6) | len(body))
    else:
        header = struct.pack("<HI", (tag_type << 6) | 0x3f, len(body))
    return header + body


def encode_rect(xmin: int, xmax: int, ymin: int, ymax: int) -> bytes:
    def bits_needed(v: int) -> int:
        if v == 0:
            return 0
        if v > 0:
            return v.bit_length() + 1
        return (-v - 1).bit_length() + 1

    nbits = max(bits_needed(v) for v in (xmin, xmax, ymin, ymax)) or 1

    bit_str = format(nbits, "05b")
    for v in (xmin, xmax, ymin, ymax):
        if v < 0:
            v = (1 << nbits) + v
        bit_str += format(v, f"0{nbits}b")
    while len(bit_str) % 8:
        bit_str += "0"
    return bytes(int(bit_str[i:i + 8], 2) for i in range(0, len(bit_str), 8))


def pack_pixels_argb_premultiplied(image: Image.Image) -> bytes:
    rgba = image.convert("RGBA")
    r, g, b, a = rgba.split()
    extrema = a.getextrema()

    if extrema == (255, 255):
        # Opaque — premultiplication is identity. Reorder channels into ARGB
        # bytes via Pillow's merge (mode stays "RGBA" but byte sequence is ARGB).
        return Image.merge("RGBA", (a, r, g, b)).tobytes()

    src = bytearray(rgba.tobytes())
    n = len(src) // 4
    out = bytearray(n * 4)
    for i in range(n):
        rr = src[i * 4 + 0]
        gg = src[i * 4 + 1]
        bb = src[i * 4 + 2]
        aa = src[i * 4 + 3]
        out[i * 4 + 0] = aa
        out[i * 4 + 1] = (rr * aa) // 255
        out[i * 4 + 2] = (gg * aa) // 255
        out[i * 4 + 3] = (bb * aa) // 255
    return bytes(out)


def make_define_bits_lossless2(char_id: int, image: Image.Image) -> bytes:
    width, height = image.size
    pixels = pack_pixels_argb_premultiplied(image)
    compressed = zlib.compress(pixels, level=9)
    body = struct.pack("<HBHH", char_id, 5, width, height) + compressed
    return encode_tag(TAG_DEFINE_BITS_LOSSLESS2, body)


def make_symbol_class(symbols: list[tuple[int, str]]) -> bytes:
    body = struct.pack("<H", len(symbols))
    for char_id, name in symbols:
        body += struct.pack("<H", char_id) + name.encode("ascii") + b"\x00"
    return encode_tag(TAG_SYMBOL_CLASS, body)


def make_file_attributes(uses_as3: bool = True) -> bytes:
    # Byte 0 bit 3 = ActionScript3. Required as the FIRST tag in SWF 8+ for AS3.
    flags = 0x08 if uses_as3 else 0
    return encode_tag(TAG_FILE_ATTRIBUTES, struct.pack("<I", flags))


def make_set_background_color(rgb: tuple[int, int, int]) -> bytes:
    return encode_tag(TAG_SET_BACKGROUND_COLOR, bytes(rgb))


def make_show_frame() -> bytes:
    return encode_tag(TAG_SHOW_FRAME, b"")


def make_end() -> bytes:
    return encode_tag(TAG_END, b"")


def build_swf(image: Image.Image, class_name: str) -> bytes:
    width, height = image.size
    body = b""
    body += encode_rect(0, width * TWIPS, 0, height * TWIPS)
    body += struct.pack("<H", 24 << 8)  # 24 fps (8.8 fixed-point)
    body += struct.pack("<H", 1)        # 1 frame

    body += make_file_attributes(uses_as3=True)
    body += make_set_background_color((0, 0, 0))
    body += make_define_bits_lossless2(1, image)
    body += make_symbol_class([(1, class_name)])
    body += make_show_frame()
    body += make_end()

    file_length = 8 + len(body)
    header = b"FWS" + bytes([SWF_VERSION]) + struct.pack("<I", file_length)
    return header + body


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--input", default=str(DEFAULT_INPUT))
    p.add_argument("--output", default=str(DEFAULT_OUTPUT))
    p.add_argument("--class-name", default="s1")
    p.add_argument("--max-width", type=int, default=720,
                   help="Downscale to this max width; 0 disables resizing.")
    args = p.parse_args()

    src = Path(args.input)
    if not src.is_file():
        print(f"input not found: {src}", file=sys.stderr)
        return 1

    img = Image.open(src)
    print(f"loaded {src.name}: {img.width}x{img.height} mode={img.mode}")

    if args.max_width > 0 and img.width > args.max_width:
        scale = args.max_width / img.width
        img = img.resize((args.max_width, int(round(img.height * scale))), Image.LANCZOS)
        print(f"resized to {img.width}x{img.height}")

    swf = build_swf(img, args.class_name)

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(swf)
    print(f"wrote {out}: {len(swf):,} bytes (class='{args.class_name}')")
    return 0


if __name__ == "__main__":
    sys.exit(main())
