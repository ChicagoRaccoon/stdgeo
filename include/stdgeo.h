#ifndef STDGEO_H
#define STDGEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// Opaque types
typedef struct Point3D Point3D;
typedef struct Vector3D Vector3D;
typedef struct Triangle Triangle;
typedef struct Mesh Mesh;

// Point3D functions
Point3D* point3d_new(double x, double y, double z);
void point3d_free(Point3D* point);
double point3d_get_x(const Point3D* point);
double point3d_get_y(const Point3D* point);
double point3d_get_z(const Point3D* point);
double point3d_distance_to(const Point3D* point1, const Point3D* point2);

// Vector3D functions
Vector3D* vector3d_new(double x, double y, double z);
void vector3d_free(Vector3D* vector);
double vector3d_magnitude(const Vector3D* vector);
void vector3d_normalize(Vector3D* vector);
double vector3d_dot(const Vector3D* v1, const Vector3D* v2);

// Triangle functions
Triangle* triangle_new(const Point3D* v0, const Point3D* v1, const Point3D* v2);
void triangle_free(Triangle* triangle);
double triangle_area(const Triangle* triangle);

// Mesh functions
Mesh* mesh_new(void);
void mesh_free(Mesh* mesh);
Mesh* mesh_create_cube(double size);
size_t mesh_vertex_count(const Mesh* mesh);
size_t mesh_triangle_count(const Mesh* mesh);
int mesh_get_vertex(const Mesh* mesh, size_t index, double* x, double* y, double* z);
int mesh_get_triangle_vertices(
    const Mesh* mesh, 
    size_t triangle_index,
    double* v0_x, double* v0_y, double* v0_z,
    double* v1_x, double* v1_y, double* v1_z,
    double* v2_x, double* v2_y, double* v2_z
);

#ifdef __cplusplus
}
#endif

#endif // STDGEO_H