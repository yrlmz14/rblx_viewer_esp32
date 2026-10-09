#ifndef RENDERER3D_H
#define RENDERER3D_H

#include <vector>
#include <algorithm>
#include "math3d.h"
#include "r3d_parser.h"

struct Triangle3D {
    Vector3 v0, v1, v2;
    Vector3 normal;
    uint16_t color;
    float avgZ;
};

class Renderer3D {
public:
    Renderer3D(int width, int height);

    void setCamera(float rotX, float rotY, float zoom);
    void renderParts(const std::vector<R3DPart>& parts, uint16_t* frameBuffer);

private:
    int screenWidth;
    int screenHeight;
    float camRotX;
    float camRotY;
    float camZoom;

    Vector3 lightDir;

    void generateBoxMesh(const R3DPart& part, std::vector<Triangle3D>& outTris);
    void projectToScreen(const Vector3& p, int& scX, int& scY);
    void drawTriangleFlat(uint16_t* fb, int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
};

#endif // RENDERER3D_H
