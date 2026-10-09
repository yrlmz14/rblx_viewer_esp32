#ifndef R3D_PARSER_H
#define R3D_PARSER_H

#include <stdint.h>
#include <stddef.h>
#include <vector>

enum R3DShapeType {
    R3D_SHAPE_BLOCK = 0,
    R3D_SHAPE_SPHERE = 1,
    R3D_SHAPE_CYLINDER = 2,
    R3D_SHAPE_WEDGE = 3,
    R3D_SHAPE_CORNERWEDGE = 4
};

struct R3DPart {
    uint8_t shape;
    float posX, posY, posZ;
    float sizeX, sizeY, sizeZ;
    float rot[9]; // 3x3 rotation matrix
    uint16_t colorRGB565;
    float transparency;
};

struct R3DHeader {
    char magic[4];    // "R3D1"
    uint16_t version; // 1
    uint32_t partCount;
};

class R3DParser {
public:
    static bool parseFromBuffer(const uint8_t* buffer, size_t size, std::vector<R3DPart>& outParts);
    static bool parseFromFile(const char* filepath, std::vector<R3DPart>& outParts);
};

#endif // R3D_PARSER_H
