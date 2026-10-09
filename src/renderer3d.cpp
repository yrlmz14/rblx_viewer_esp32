#include "renderer3d.h"

Renderer3D::Renderer3D(int width, int height)
    : screenWidth(width), screenHeight(height), camRotX(0.4f), camRotY(0.6f), camZoom(25.0f) {
    lightDir = Vector3(0.577f, 0.577f, -0.577f).normalized();
}

void Renderer3D::setCamera(float rotX, float rotY, float zoom) {
    camRotX = rotX;
    camRotY = rotY;
    camZoom = zoom;
}

void Renderer3D::generateBoxMesh(const R3DPart& part, std::vector<Triangle3D>& outTris) {
    float hx = part.sizeX * 0.5f;
    float hy = part.sizeY * 0.5f;
    float hz = part.sizeZ * 0.5f;

    Vector3 localVertices[8] = {
        {-hx, -hy, -hz}, { hx, -hy, -hz}, { hx,  hy, -hz}, {-hx,  hy, -hz},
        {-hx, -hy,  hz}, { hx, -hy,  hz}, { hx,  hy,  hz}, {-hx,  hy,  hz}
    };

    // Transform by CFrame rot matrix and position
    Vector3 worldVerts[8];
    for (int i = 0; i < 8; ++i) {
        float x = localVertices[i].x;
        float y = localVertices[i].y;
        float z = localVertices[i].z;

        float wx = part.rot[0]*x + part.rot[1]*y + part.rot[2]*z + part.posX;
        float wy = part.rot[3]*x + part.rot[4]*y + part.rot[5]*z + part.posY;
        float wz = part.rot[6]*x + part.rot[7]*y + part.rot[8]*z + part.posZ;

        worldVerts[i] = Vector3(wx, wy, wz);
    }

    static const int faceIndices[6][4] = {
        {0, 1, 2, 3}, // Back
        {5, 4, 7, 6}, // Front
        {4, 0, 3, 7}, // Left
        {1, 5, 6, 2}, // Right
        {4, 5, 1, 0}, // Bottom
        {3, 2, 6, 7}  // Top
    };

    for (int f = 0; f < 6; ++f) {
        Vector3 v0 = worldVerts[faceIndices[f][0]];
        Vector3 v1 = worldVerts[faceIndices[f][1]];
        Vector3 v2 = worldVerts[faceIndices[f][2]];
        Vector3 v3 = worldVerts[faceIndices[f][3]];

        Vector3 normal = (v1 - v0).cross(v2 - v0).normalized();
        float intensity = std::max(0.2f, normal.dot(lightDir));

        // Unpack RGB565 and apply shading
        uint16_t c565 = part.colorRGB565;
        uint8_t r = ((c565 >> 11) & 0x1F) << 3;
        uint8_t g = ((c565 >> 5) & 0x3F) << 2;
        uint8_t b = (c565 & 0x1F) << 3;

        r = (uint8_t)(r * intensity);
        g = (uint8_t)(g * intensity);
        b = (uint8_t)(b * intensity);

        uint16_t shadedColor = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        Triangle3D t1 = {v0, v1, v2, normal, shadedColor, 0.0f};
        Triangle3D t2 = {v0, v2, v3, normal, shadedColor, 0.0f};

        outTris.push_back(t1);
        outTris.push_back(t2);
    }
}

void Renderer3D::projectToScreen(const Vector3& p, int& scX, int& scY) {
    float aspect = (float)screenWidth / (float)screenHeight;
    Matrix4 proj = Matrix4::perspective(0.785398f, aspect, 0.1f, 1000.0f); // 45 deg FOV
    Vector3 projP = proj.transformPoint(p);

    scX = (int)((projP.x + 1.0f) * 0.5f * screenWidth);
    scY = (int)((1.0f - projP.y) * 0.5f * screenHeight);
}

void Renderer3D::drawTriangleFlat(uint16_t* fb, int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color) {
    // Simple wireframe / bounding triangle rasterization logic
    if (!fb) return;
    int minX = std::max(0, std::min({x0, x1, x2}));
    int maxX = std::min(screenWidth - 1, std::max({x0, x1, x2}));
    int minY = std::max(0, std::min({y0, y1, y2}));
    int maxY = std::min(screenHeight - 1, std::max({y0, y1, y2}));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            // Edge function for barycentric rasterization
            int w0 = (x1 - x0)*(y - y0) - (y1 - y0)*(x - x0);
            int w1 = (x2 - x1)*(y - y1) - (y2 - y1)*(x - x1);
            int w2 = (x0 - x2)*(y - y2) - (y0 - y2)*(x - x2);

            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                fb[y * screenWidth + x] = color;
            }
        }
    }
}

void Renderer3D::renderParts(const std::vector<R3DPart>& parts, uint16_t* frameBuffer) {
    std::vector<Triangle3D> triList;
    for (const auto& part : parts) {
        generateBoxMesh(part, triList);
    }

    Matrix4 rotXMat = Matrix4::rotationX(camRotX);
    Matrix4 rotYMat = Matrix4::rotationY(camRotY);
    Matrix4 transMat = Matrix4::translation(0, 0, -camZoom);

    // Transform vertices to view space & calculate depth sort key
    for (auto& tri : triList) {
        Vector3 v0_cam = transMat.transformPoint(rotXMat.transformPoint(rotYMat.transformPoint(tri.v0)));
        Vector3 v1_cam = transMat.transformPoint(rotXMat.transformPoint(rotYMat.transformPoint(tri.v1)));
        Vector3 v2_cam = transMat.transformPoint(rotXMat.transformPoint(rotYMat.transformPoint(tri.v2)));

        tri.v0 = v0_cam;
        tri.v1 = v1_cam;
        tri.v2 = v2_cam;
        tri.avgZ = (v0_cam.z + v1_cam.z + v2_cam.z) / 3.0f;
    }

    // Painter's algorithm sort back-to-front
    std::sort(triList.begin(), triList.end(), [](const Triangle3D& a, const Triangle3D& b) {
        return a.avgZ < b.avgZ;
    });

    // Render triangles
    for (const auto& tri : triList) {
        if (tri.v0.z >= -0.1f || tri.v1.z >= -0.1f || tri.v2.z >= -0.1f) continue; // Near plane clip

        int x0, y0, x1, y1, x2, y2;
        projectToScreen(tri.v0, x0, y0);
        projectToScreen(tri.v1, x1, y1);
        projectToScreen(tri.v2, x2, y2);

        drawTriangleFlat(frameBuffer, x0, y0, x1, y1, x2, y2, tri.color);
    }
}
