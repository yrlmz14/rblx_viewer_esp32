#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "math3d.h"
#include "r3d_parser.h"

void test_vector3_operations() {
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);

    Vector3 sum = a + b;
    assert(std::abs(sum.x - 5.0f) < 0.001f);
    assert(std::abs(sum.y - 7.0f) < 0.001f);
    assert(std::abs(sum.z - 9.0f) < 0.001f);

    float dot = a.dot(b);
    assert(std::abs(dot - 32.0f) < 0.001f);
    std::cout << "[PASS] Vector3 operations\n";
}

void test_matrix_transform() {
    Matrix4 trans = Matrix4::translation(10.0f, -5.0f, 2.0f);
    Vector3 p(0.0f, 0.0f, 0.0f);
    Vector3 result = trans.transformPoint(p);

    assert(std::abs(result.x - 10.0f) < 0.001f);
    assert(std::abs(result.y - (-5.0f)) < 0.001f);
    assert(std::abs(result.z - 2.0f) < 0.001f);
    std::cout << "[PASS] Matrix4 transformations\n";
}

void test_r3d_parser_invalid_header() {
    uint8_t dummy[16] = {0};
    std::vector<R3DPart> parts;
    bool status = R3DParser::parseFromBuffer(dummy, sizeof(dummy), parts);
    assert(!status);
    std::cout << "[PASS] R3DParser header validation\n";
}

int main() {
    std::cout << "Running unit tests...\n";
    test_vector3_operations();
    test_matrix_transform();
    test_r3d_parser_invalid_header();
    std::cout << "All unit tests passed successfully!\n";
    return 0;
}
