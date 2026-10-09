#!/usr/bin/env python3
"""
Roblox Place (.rbxl / .rbxlx) to ESP32 Optimized Binary (.r3d) Converter
Converts Roblox place files containing Part instances into an optimized
3D mesh binary format for instant loading on ESP32-S3.

Format Specification (.r3d Header + Models):
- Magic: "R3D1" (4 bytes)
- Version: uint16 (2 bytes) -> 1
- Part Count: uint32 (4 bytes)
- Part Entry (variable per part):
  - Shape Type: uint8 (0=Block, 1=Sphere, 2=Cylinder, 3=Wedge, 4=CornerWedge)
  - Position: 3 x float32 (X, Y, Z)
  - Size: 3 x float32 (X, Y, Z)
  - Rotation Matrix / Quaternion / CFrame: 9 x float32 (R00, R01, R02, R10, R11, R12, R20, R21, R22)
  - Color: uint32 (0xRRGGBBAA or 0x565 RGB)
  - Transparency: float32 (0.0 to 1.0)
"""

import sys
import os
import struct
import xml.etree.ElementTree as ET

SHAPE_BLOCK = 0
SHAPE_SPHERE = 1
SHAPE_CYLINDER = 2
SHAPE_WEDGE = 3
SHAPE_CORNERWEDGE = 4

SHAPE_NAME_MAP = {
    '0': SHAPE_BLOCK,
    '1': SHAPE_CYLINDER,
    '2': SHAPE_SPHERE,
    'block': SHAPE_BLOCK,
    'sphere': SHAPE_SPHERE,
    'cylinder': SHAPE_CYLINDER,
    'wedge': SHAPE_WEDGE,
    'cornerwedge': SHAPE_CORNERWEDGE
}

def parse_cframe(cframe_elem):
    if cframe_elem is None:
        return (0.0, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)

    pos = [0.0, 0.0, 0.0]
    rot = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]

    for child in cframe_elem:
        tag = child.tag.upper()
        val = float(child.text) if child.text else 0.0
        if tag == 'X': pos[0] = val
        elif tag == 'Y': pos[1] = val
        elif tag == 'Z': pos[2] = val
        elif tag == 'R00': rot[0] = val
        elif tag == 'R01': rot[1] = val
        elif tag == 'R02': rot[2] = val
        elif tag == 'R10': rot[3] = val
        elif tag == 'R11': rot[4] = val
        elif tag == 'R12': rot[5] = val
        elif tag == 'R20': rot[6] = val
        elif tag == 'R21': rot[7] = val
        elif tag == 'R22': rot[8] = val

    return tuple(pos), tuple(rot)

def parse_vector3(vec_elem, default=(1.0, 1.0, 1.0)):
    if vec_elem is None:
        return default
    v = list(default)
    for child in vec_elem:
        tag = child.tag.upper()
        val = float(child.text) if child.text else 0.0
        if tag == 'X': v[0] = val
        elif tag == 'Y': v[1] = val
        elif tag == 'Z': v[2] = val
    return tuple(v)

def parse_color3(color_elem, default_rgb=(163, 162, 165)):
    if color_elem is None:
        return default_rgb

    # Check if text exists like 0xFFFFFFFF or integer color33
    if color_elem.text and color_elem.text.strip():
        try:
            val = int(color_elem.text.strip())
            r = (val >> 16) & 0xFF
            g = (val >> 8) & 0xFF
            b = val & 0xFF
            return (r, g, b)
        except ValueError:
            pass

    r, g, b = default_rgb
    for child in color_elem:
        tag = child.tag.upper()
        val = float(child.text) if child.text else 0.0
        # If color values are in 0.0-1.0 range
        if val <= 1.0: val = val * 255.0
        if tag == 'R': r = int(val)
        elif tag == 'G': g = int(val)
        elif tag == 'B': b = int(val)
    return (max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b)))

def parse_rbxlx(xml_path):
    tree = ET.parse(xml_path)
    root = tree.getroot()
    parts = []

    for item in root.iter('Item'):
        class_name = item.get('class', '')
        if class_name in ('Part', 'WedgePart', 'CornerWedgePart', 'MeshPart', 'TrussPart'):
            shape = SHAPE_BLOCK
            if class_name == 'WedgePart': shape = SHAPE_WEDGE
            elif class_name == 'CornerWedgePart': shape = SHAPE_CORNERWEDGE

            cframe_pos = (0.0, 0.0, 0.0)
            cframe_rot = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)
            size = (4.0, 1.2, 2.0)
            color = (163, 162, 165)
            transparency = 0.0

            properties = item.find('Properties')
            if properties is not None:
                for prop in properties:
                    name = prop.get('name', '')
                    if name in ('CFrame', 'CoordinateFrame'):
                        cframe_pos, cframe_rot = parse_cframe(prop)
                    elif name == 'size':
                        size = parse_vector3(prop, default=(4.0, 1.2, 2.0))
                    elif name in ('Color3uint8', 'Color3', 'Color'):
                        color = parse_color3(prop)
                    elif name == 'shape':
                        s_text = prop.text.strip() if prop.text else '0'
                        shape = SHAPE_NAME_MAP.get(s_text.lower(), SHAPE_BLOCK)
                    elif name == 'Transparency':
                        try:
                            transparency = float(prop.text) if prop.text else 0.0
                        except ValueError:
                            transparency = 0.0

            parts.append({
                'shape': shape,
                'pos': cframe_pos,
                'size': size,
                'rot': cframe_rot,
                'color': color,
                'transparency': transparency
            })
    return parts

def convert_to_r3d(parts, output_path):
    with open(output_path, 'wb') as f:
        # Header: Magic "R3D1", Version=1, PartCount
        f.write(b'R3D1')
        f.write(struct.pack('<H', 1))
        f.write(struct.pack('<I', len(parts)))

        for p in parts:
            # Shape (uint8)
            f.write(struct.pack('<B', p['shape']))
            # Position (3 floats)
            f.write(struct.pack('<3f', *p['pos']))
            # Size (3 floats)
            f.write(struct.pack('<3f', *p['size']))
            # Rotation Matrix (9 floats)
            f.write(struct.pack('<9f', *p['rot']))
            # Color RGB565 or RGBA
            r, g, b = p['color']
            # RGB565 pack
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            f.write(struct.pack('<H', rgb565))
            # Transparency (float)
            f.write(struct.pack('<f', p['transparency']))

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 rbxl_convert.py <input.rbxlx / input.rbxl> [output.r3d]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else os.path.splitext(input_path)[0] + '.r3d'

    if not os.path.exists(input_path):
        print(f"Error: File {input_path} does not exist.")
        sys.exit(1)

    print(f"Parsing Roblox place file: {input_path} ...")
    parts = parse_rbxlx(input_path)
    print(f"Parsed {len(parts)} Roblox part(s).")

    print(f"Converting to R3D binary format: {output_path} ...")
    convert_to_r3d(parts, output_path)
    print(f"Successfully converted -> {output_path}")

if __name__ == '__main__':
    main()
