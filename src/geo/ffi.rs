use crate::geometry::*;
use libc::{c_double, c_int, size_t};
use std::ptr;

// Point3D FFI functions
#[no_mangle]
pub extern "C" fn point3d_new(x: c_double, y: c_double, z: c_double) -> *mut Point3D {
    Box::into_raw(Box::new(Point3D::new(x, y, z)))
}

#[no_mangle]
pub extern "C" fn point3d_free(point: *mut Point3D) {
    if !point.is_null() {
        unsafe { let _ = Box::from_raw(point); };
    }
}

#[no_mangle]
pub extern "C" fn point3d_get_x(point: *const Point3D) -> c_double {
    if point.is_null() { return 0.0; }
    unsafe { (*point).x }
}

#[no_mangle]
pub extern "C" fn point3d_get_y(point: *const Point3D) -> c_double {
    if point.is_null() { return 0.0; }
    unsafe { (*point).y }
}

#[no_mangle]
pub extern "C" fn point3d_get_z(point: *const Point3D) -> c_double {
    if point.is_null() { return 0.0; }
    unsafe { (*point).z }
}

#[no_mangle]
pub extern "C" fn point3d_distance_to(point1: *const Point3D, point2: *const Point3D) -> c_double {
    if point1.is_null() || point2.is_null() { return 0.0; }
    unsafe { (*point1).distance_to(&*point2) }
}

// Vector3D FFI functions
#[no_mangle]
pub extern "C" fn vector3d_new(x: c_double, y: c_double, z: c_double) -> *mut Vector3D {
    Box::into_raw(Box::new(Vector3D::new(x, y, z)))
}

#[no_mangle]
pub extern "C" fn vector3d_free(vector: *mut Vector3D) {
    if !vector.is_null() {
        unsafe { let _ = Box::from_raw(vector); };
    }
}

#[no_mangle]
pub extern "C" fn vector3d_magnitude(vector: *const Vector3D) -> c_double {
    if vector.is_null() { return 0.0; }
    unsafe { (*vector).magnitude() }
}

#[no_mangle]
pub extern "C" fn vector3d_normalize(vector: *mut Vector3D) {
    if vector.is_null() { return; }
    unsafe { (*vector).normalize() };
}

#[no_mangle]
pub extern "C" fn vector3d_dot(v1: *const Vector3D, v2: *const Vector3D) -> c_double {
    if v1.is_null() || v2.is_null() { return 0.0; }
    unsafe { (*v1).dot(&*v2) }
}

// Triangle FFI functions
#[no_mangle]
pub extern "C" fn triangle_new(v0: *const Point3D, v1: *const Point3D, v2: *const Point3D) -> *mut Triangle {
    if v0.is_null() || v1.is_null() || v2.is_null() { return ptr::null_mut(); }
    unsafe {
        Box::into_raw(Box::new(Triangle::new(*v0, *v1, *v2)))
    }
}

#[no_mangle]
pub extern "C" fn triangle_free(triangle: *mut Triangle) {
    if !triangle.is_null() {
        unsafe { let _ = Box::from_raw(triangle); };
    }
}

#[no_mangle]
pub extern "C" fn triangle_area(triangle: *const Triangle) -> c_double {
    if triangle.is_null() { return 0.0; }
    unsafe { (*triangle).area() }
}

// Mesh FFI functions
#[no_mangle]
pub extern "C" fn mesh_new() -> *mut Mesh {
    Box::into_raw(Box::new(Mesh::new()))
}

#[no_mangle]
pub extern "C" fn mesh_free(mesh: *mut Mesh) {
    if !mesh.is_null() {
        unsafe { let _ = Box::from_raw(mesh); };
    }
}

#[no_mangle]
pub extern "C" fn mesh_create_cube(size: c_double) -> *mut Mesh {
    Box::into_raw(Box::new(Mesh::create_cube(size)))
}

#[no_mangle]
pub extern "C" fn mesh_vertex_count(mesh: *const Mesh) -> size_t {
    if mesh.is_null() { return 0; }
    unsafe { (*mesh).vertex_count() }
}

#[no_mangle]
pub extern "C" fn mesh_triangle_count(mesh: *const Mesh) -> size_t {
    if mesh.is_null() { return 0; }
    unsafe { (*mesh).triangle_count() }
}

#[no_mangle]
pub extern "C" fn mesh_get_vertex(mesh: *const Mesh, index: size_t, x: *mut c_double, y: *mut c_double, z: *mut c_double) -> c_int {
    if mesh.is_null() || x.is_null() || y.is_null() || z.is_null() { return 0; }
    unsafe {
        let mesh_ref = &*mesh;
        if index >= mesh_ref.vertices.len() { return 0; }
        let vertex = &mesh_ref.vertices[index];
        *x = vertex.x;
        *y = vertex.y;
        *z = vertex.z;
        1
    }
}

#[no_mangle]
pub extern "C" fn mesh_get_triangle_vertices(
    mesh: *const Mesh, 
    triangle_index: size_t,
    v0_x: *mut c_double, v0_y: *mut c_double, v0_z: *mut c_double,
    v1_x: *mut c_double, v1_y: *mut c_double, v1_z: *mut c_double,
    v2_x: *mut c_double, v2_y: *mut c_double, v2_z: *mut c_double
) -> c_int {
    if mesh.is_null() { return 0; }
    unsafe {
        let mesh_ref = &*mesh;
        if triangle_index >= mesh_ref.triangles.len() { return 0; }
        let triangle = &mesh_ref.triangles[triangle_index];
        
        if !v0_x.is_null() { *v0_x = triangle.v0.x; }
        if !v0_y.is_null() { *v0_y = triangle.v0.y; }
        if !v0_z.is_null() { *v0_z = triangle.v0.z; }
        
        if !v1_x.is_null() { *v1_x = triangle.v1.x; }
        if !v1_y.is_null() { *v1_y = triangle.v1.y; }
        if !v1_z.is_null() { *v1_z = triangle.v1.z; }
        
        if !v2_x.is_null() { *v2_x = triangle.v2.x; }
        if !v2_y.is_null() { *v2_y = triangle.v2.y; }
        if !v2_z.is_null() { *v2_z = triangle.v2.z; }
        
        1
    }
}

#[cfg(test)]
mod ffi_tests {
    use super::*;
    use std::ptr;

    #[test]
    fn test_point3d_ffi_lifecycle() {
        let point = point3d_new(1.0, 2.0, 3.0);
        assert!(!point.is_null());
        
        assert_eq!(point3d_get_x(point), 1.0);
        assert_eq!(point3d_get_y(point), 2.0);
        assert_eq!(point3d_get_z(point), 3.0);
        
        point3d_free(point);
    }

    #[test]
    fn test_point3d_ffi_null_safety() {
        assert_eq!(point3d_get_x(ptr::null()), 0.0);
        assert_eq!(point3d_get_y(ptr::null()), 0.0);
        assert_eq!(point3d_get_z(ptr::null()), 0.0);
        
        point3d_free(ptr::null_mut());
    }

    #[test]
    fn test_mesh_create_cube_ffi() {
        let cube = mesh_create_cube(2.0);
        assert!(!cube.is_null());
        
        assert_eq!(mesh_vertex_count(cube), 8);
        assert_eq!(mesh_triangle_count(cube), 12);
        
        let mut x = 0.0; let mut y = 0.0; let mut z = 0.0;
        let success = mesh_get_vertex(cube, 0, &mut x, &mut y, &mut z);
        assert_eq!(success, 1);
        assert_eq!(x, -1.0);
        assert_eq!(y, -1.0);
        assert_eq!(z, -1.0);
        
        mesh_free(cube);
    }
}