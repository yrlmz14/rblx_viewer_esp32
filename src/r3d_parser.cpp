#include "r3d_parser.h"
#include <cstdio>
#include <cstring>

bool R3DParser::parseFromBuffer(const uint8_t* buffer, size_t size, std::vector<R3DPart>& outParts) {
    if (!buffer || size < sizeof(R3DHeader)) return false;

    const R3DHeader* header = reinterpret_cast<const R3DHeader*>(buffer);
    if (std::memcmp(header->magic, "R3D1", 4) != 0) return false;

    size_t offset = sizeof(R3DHeader);
    outParts.reserve(header->partCount);

    for (uint32_t i = 0; i < header->partCount; ++i) {
        if (offset + 1 + 3*4 + 3*4 + 9*4 + 2 + 4 > size) break;

        R3DPart part;
        part.shape = buffer[offset++];

        std::memcpy(&part.posX, buffer + offset, 4); offset += 4;
        std::memcpy(&part.posY, buffer + offset, 4); offset += 4;
        std::memcpy(&part.posZ, buffer + offset, 4); offset += 4;

        std::memcpy(&part.sizeX, buffer + offset, 4); offset += 4;
        std::memcpy(&part.sizeY, buffer + offset, 4); offset += 4;
        std::memcpy(&part.sizeZ, buffer + offset, 4); offset += 4;

        for (int r = 0; r < 9; ++r) {
            std::memcpy(&part.rot[r], buffer + offset, 4);
            offset += 4;
        }

        std::memcpy(&part.colorRGB565, buffer + offset, 2); offset += 2;
        std::memcpy(&part.transparency, buffer + offset, 4); offset += 4;

        outParts.push_back(part);
    }

    return true;
}

bool R3DParser::parseFromFile(const char* filepath, std::vector<R3DPart>& outParts) {
    FILE* f = std::fopen(filepath, "rb");
    if (!f) return false;

    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    if (sz < (long)sizeof(R3DHeader)) {
        std::fclose(f);
        return false;
    }

    std::vector<uint8_t> buf(sz);
    std::fread(buf.data(), 1, sz, f);
    std::fclose(f);

    return parseFromBuffer(buf.data(), buf.size(), outParts);
}
