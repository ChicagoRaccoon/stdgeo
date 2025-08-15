#include <gtest/gtest.h>
#include "stdgeo.h"

class FFIIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }
    
    void TearDown() override {
        // Cleanup code
    }
};

// Point3D FFI Tests
TEST_F(FFIIntegrationTest, Point3DBasicOperations) {
    Point3D* point = point3d_new(1.0, 2.0, 3.0);
    ASSERT_NE(point, nullptr);
    
    EXPECT_EQ(point3d_get_x(point), 1.0);
    EXPECT_EQ(point3d_get_y(point), 2.0);
    EXPECT_EQ(point3d_get_z(point), 3.0);
    
    point3d_free(point);
}

TEST_F(FFIIntegrationTest, Point3DDistance) {
    Point3D* p1 = point3d_new(0.0, 0.0, 0.0);
    Point3D* p2 = point3d_new(3.0, 4.0, 0.0);
    
    double distance = point3d_distance_to(p1, p2);
    EXPECT_EQ(distance, 5.0);
    
    point3d_free(p1);
    point3d_free(p2);
}

TEST_F(FFIIntegrationTest, Point3DNullSafety) {
    EXPECT_EQ(point3d_get_x(nullptr), 0.0);
    EXPECT_EQ(point3d_get_y(nullptr), 0.0);
    EXPECT_EQ(point3d_get_z(nullptr), 0.0);
    
    // Should not crash
    point3d_free(nullptr);
}

// Vector3D FFI Tests
TEST_F(FFIIntegrationTest, Vector3DBasicOperations) {
    Vector3D* vector = vector3d_new(3.0, 4.0, 0.0);
    ASSERT_NE(vector, nullptr);
    
    double magnitude = vector3d_magnitude(vector);
    EXPECT_EQ(magnitude, 5.0);
    
    vector3d_normalize(vector);
    double new_magnitude = vector3d_magnitude(vector);
    EXPECT_NEAR(new_magnitude, 1.0, 1e-10);
    
    vector3d_free(vector);
}

TEST_F(FFIIntegrationTest, Vector3DDotProduct) {
    Vector3D* v1 = vector3d_new(1.0, 2.0, 3.0);
    Vector3D* v2 = vector3d_new(4.0, 5.0, 6.0);
    
    double dot = vector3d_dot(v1, v2);
    EXPECT_EQ(dot, 32.0);  // 1*4 + 2*5 + 3*6 = 32
    
    vector3d_free(v1);
    vector3d_free(v2);
}

// Triangle FFI Tests
TEST_F(FFIIntegrationTest, TriangleOperations) {
    Point3D* v0 = point3d_new(0.0, 0.0, 0.0);
    Point3D* v1 = point3d_new(2.0, 0.0, 0.0);
    Point3D* v2 = point3d_new(0.0, 2.0, 0.0);
    
    Triangle* triangle = triangle_new(v0, v1, v2);
    ASSERT_NE(triangle, nullptr);
    
    double area = triangle_area(triangle);
    EXPECT_EQ(area, 2.0);  // Right triangle with legs of length 2
    
    triangle_free(triangle);
    point3d_free(v0);
    point3d_free(v1);
    point3d_free(v2);
}

// Mesh FFI Tests
TEST_F(FFIIntegrationTest, MeshBasicOperations) {
    Mesh* mesh = mesh_new();
    ASSERT_NE(mesh, nullptr);
    
    EXPECT_EQ(mesh_vertex_count(mesh), 0);
    EXPECT_EQ(mesh_triangle_count(mesh), 0);
    
    mesh_free(mesh);
}

TEST_F(FFIIntegrationTest, MeshCreateCube) {
    Mesh* cube = mesh_create_cube(2.0);
    ASSERT_NE(cube, nullptr);
    
    EXPECT_EQ(mesh_vertex_count(cube), 8);
    EXPECT_EQ(mesh_triangle_count(cube), 12);
    
    // Test vertex access
    double x, y, z;
    int success = mesh_get_vertex(cube, 0, &x, &y, &z);
    EXPECT_EQ(success, 1);
    EXPECT_EQ(x, -1.0);
    EXPECT_EQ(y, -1.0);
    EXPECT_EQ(z, -1.0);
    
    // Test out of bounds
    success = mesh_get_vertex(cube, 100, &x, &y, &z);
    EXPECT_EQ(success, 0);
    
    mesh_free(cube);
}

TEST_F(FFIIntegrationTest, MeshTriangleVertices) {
    Mesh* cube = mesh_create_cube(2.0);
    
    double v0_x, v0_y, v0_z, v1_x, v1_y, v1_z, v2_x, v2_y, v2_z;
    
    int success = mesh_get_triangle_vertices(
        cube, 0,
        &v0_x, &v0_y, &v0_z,
        &v1_x, &v1_y, &v1_z,
        &v2_x, &v2_y, &v2_z
    );
    EXPECT_EQ(success, 1);
    
    // Verify coordinates are within cube bounds
    EXPECT_GE(v0_x, -1.0); EXPECT_LE(v0_x, 1.0);
    EXPECT_GE(v0_y, -1.0); EXPECT_LE(v0_y, 1.0);
    EXPECT_GE(v0_z, -1.0); EXPECT_LE(v0_z, 1.0);
    
    mesh_free(cube);
}

TEST_F(FFIIntegrationTest, MeshNullSafety) {
    EXPECT_EQ(mesh_vertex_count(nullptr), 0);
    EXPECT_EQ(mesh_triangle_count(nullptr), 0);
    
    double x, y, z;
    EXPECT_EQ(mesh_get_vertex(nullptr, 0, &x, &y, &z), 0);
    
    // Should not crash
    mesh_free(nullptr);
}