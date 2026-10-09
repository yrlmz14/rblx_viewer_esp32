#include <unity.h>
#include "math3d.h"
#include "r3d_parser.h"

void setUp(void) {}
void tearDown(void) {}

void test_vector3_operations(void) {
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);

    Vector3 sum = a + b;
    TEST_ASSERT_EQUAL_FLOAT(5.0f, sum.x);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, sum.y);
    TEST_ASSERT_EQUAL_FLOAT(9.0f, sum.z);

    float dot = a.dot(b);
    TEST_ASSERT_EQUAL_FLOAT(32.0f, dot);
}

void test_matrix_transform(void) {
    Matrix4 trans = Matrix4::translation(10.0f, -5.0f, 2.0f);
    Vector3 p(0.0f, 0.0f, 0.0f);
    Vector3 result = trans.transformPoint(p);

    TEST_ASSERT_EQUAL_FLOAT(10.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(-5.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, result.z);
}

void test_r3d_parser_invalid_header(void) {
    uint8_t dummy[16] = {0};
    std::vector<R3DPart> parts;
    bool status = R3DParser::parseFromBuffer(dummy, sizeof(dummy), parts);
    TEST_ASSERT_FALSE(status);
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_vector3_operations);
    RUN_TEST(test_matrix_transform);
    RUN_TEST(test_r3d_parser_invalid_header);
    return UNITY_END();
}

int main(void) {
    return runUnityTests();
}
