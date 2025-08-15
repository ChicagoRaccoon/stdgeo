use std::fmt;

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Point3D {
    pub x: f64,
    pub y: f64,
    pub z: f64,
}

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Vector3D {
    pub x: f64,
    pub y: f64,
    pub z: f64,
}

#[repr(C)]
#[derive(Debug, Clone)]
pub struct Triangle {
    pub v0: Point3D,
    pub v1: Point3D,
    pub v2: Point3D,
}

#[derive(Debug, Clone)]
pub struct Mesh {
    pub vertices: Vec<Point3D>,
    pub triangles: Vec<Triangle>,
}

impl Point3D {
    pub fn new(x: f64, y: f64, z: f64) -> Self {
        Point3D { x, y, z }
    }

    pub fn origin() -> Self {
        Point3D::new(0.0, 0.0, 0.0)
    }

    pub fn distance_to(&self, other: &Point3D) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        let dz = self.z - other.z;
        (dx * dx + dy * dy + dz * dz).sqrt()
    }

    pub fn translate(&mut self, vector: &Vector3D) {
        self.x += vector.x;
        self.y += vector.y;
        self.z += vector.z;
    }
}

impl Vector3D {
    pub fn new(x: f64, y: f64, z: f64) -> Self {
        Vector3D { x, y, z }
    }

    pub fn zero() -> Self {
        Vector3D::new(0.0, 0.0, 0.0)
    }

    pub fn magnitude(&self) -> f64 {
        (self.x * self.x + self.y * self.y + self.z * self.z).sqrt()
    }

    pub fn normalize(&mut self) {
        let mag = self.magnitude();
        if mag > 0.0 {
            self.x /= mag;
            self.y /= mag;
            self.z /= mag;
        }
    }

    pub fn dot(&self, other: &Vector3D) -> f64 {
        self.x * other.x + self.y * other.y + self.z * other.z
    }

    pub fn cross(&self, other: &Vector3D) -> Vector3D {
        Vector3D {
            x: self.y * other.z - self.z * other.y,
            y: self.z * other.x - self.x * other.z,
            z: self.x * other.y - self.y * other.x,
        }
    }
}

impl Triangle {
    pub fn new(v0: Point3D, v1: Point3D, v2: Point3D) -> Self {
        Triangle { v0, v1, v2 }
    }

    pub fn normal(&self) -> Vector3D {
        let edge1 = Vector3D::new(
            self.v1.x - self.v0.x,
            self.v1.y - self.v0.y,
            self.v1.z - self.v0.z,
        );
        let edge2 = Vector3D::new(
            self.v2.x - self.v0.x,
            self.v2.y - self.v0.y,
            self.v2.z - self.v0.z,
        );
        let mut normal = edge1.cross(&edge2);
        normal.normalize();
        normal
    }

    pub fn area(&self) -> f64 {
        let edge1 = Vector3D::new(
            self.v1.x - self.v0.x,
            self.v1.y - self.v0.y,
            self.v1.z - self.v0.z,
        );
        let edge2 = Vector3D::new(
            self.v2.x - self.v0.x,
            self.v2.y - self.v0.y,
            self.v2.z - self.v0.z,
        );
        edge1.cross(&edge2).magnitude() * 0.5
    }
}

impl Mesh {
    pub fn new() -> Self {
        Mesh {
            vertices: Vec::new(),
            triangles: Vec::new(),
        }
    }

    pub fn add_vertex(&mut self, vertex: Point3D) {
        self.vertices.push(vertex);
    }

    pub fn add_triangle(&mut self, triangle: Triangle) {
        self.triangles.push(triangle);
    }

    pub fn create_cube(size: f64) -> Self {
        let half = size / 2.0;
        let mut mesh = Mesh::new();

        // Define 8 vertices of a cube
        let vertices = [
            Point3D::new(-half, -half, -half), // 0
            Point3D::new( half, -half, -half), // 1
            Point3D::new( half,  half, -half), // 2
            Point3D::new(-half,  half, -half), // 3
            Point3D::new(-half, -half,  half), // 4
            Point3D::new( half, -half,  half), // 5
            Point3D::new( half,  half,  half), // 6
            Point3D::new(-half,  half,  half), // 7
        ];

        for vertex in &vertices {
            mesh.add_vertex(*vertex);
        }

        // Define 12 triangles (2 per face)
        let triangles = [
            // Front face
            Triangle::new(vertices[0], vertices[1], vertices[2]),
            Triangle::new(vertices[0], vertices[2], vertices[3]),
            // Back face
            Triangle::new(vertices[5], vertices[4], vertices[7]),
            Triangle::new(vertices[5], vertices[7], vertices[6]),
            // Left face
            Triangle::new(vertices[4], vertices[0], vertices[3]),
            Triangle::new(vertices[4], vertices[3], vertices[7]),
            // Right face
            Triangle::new(vertices[1], vertices[5], vertices[6]),
            Triangle::new(vertices[1], vertices[6], vertices[2]),
            // Top face
            Triangle::new(vertices[3], vertices[2], vertices[6]),
            Triangle::new(vertices[3], vertices[6], vertices[7]),
            // Bottom face
            Triangle::new(vertices[4], vertices[5], vertices[1]),
            Triangle::new(vertices[4], vertices[1], vertices[0]),
        ];

        for triangle in &triangles {
            mesh.add_triangle(triangle.clone());
        }

        mesh
    }

    pub fn vertex_count(&self) -> usize {
        self.vertices.len()
    }

    pub fn triangle_count(&self) -> usize {
        self.triangles.len()
    }
}

impl fmt::Display for Point3D {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({:.3}, {:.3}, {:.3})", self.x, self.y, self.z)
    }
}

impl fmt::Display for Vector3D {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "<{:.3}, {:.3}, {:.3}>", self.x, self.y, self.z)
    }
}

impl fmt::Display for Mesh {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Mesh: {} vertices, {} triangles", 
               self.vertex_count(), self.triangle_count())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_point3d_new() {
        let point = Point3D::new(1.0, 2.0, 3.0);
        assert_eq!(point.x, 1.0);
        assert_eq!(point.y, 2.0);
        assert_eq!(point.z, 3.0);
    }

    #[test]
    fn test_point3d_origin() {
        let origin = Point3D::origin();
        assert_eq!(origin.x, 0.0);
        assert_eq!(origin.y, 0.0);
        assert_eq!(origin.z, 0.0);
    }

    #[test]
    fn test_point3d_distance() {
        let p1 = Point3D::new(0.0, 0.0, 0.0);
        let p2 = Point3D::new(3.0, 4.0, 0.0);
        assert_eq!(p1.distance_to(&p2), 5.0);
    }

    #[test]
    fn test_point3d_translate() {
        let mut point = Point3D::new(1.0, 2.0, 3.0);
        let vector = Vector3D::new(2.0, 3.0, 4.0);
        point.translate(&vector);
        assert_eq!(point.x, 3.0);
        assert_eq!(point.y, 5.0);
        assert_eq!(point.z, 7.0);
    }

    #[test]
    fn test_vector3d_new() {
        let vector = Vector3D::new(1.0, 2.0, 3.0);
        assert_eq!(vector.x, 1.0);
        assert_eq!(vector.y, 2.0);
        assert_eq!(vector.z, 3.0);
    }

    #[test]
    fn test_vector3d_magnitude() {
        let vector = Vector3D::new(3.0, 4.0, 0.0);
        assert_eq!(vector.magnitude(), 5.0);
    }

    #[test]
    fn test_vector3d_normalize() {
        let mut vector = Vector3D::new(3.0, 4.0, 0.0);
        vector.normalize();
        assert!((vector.magnitude() - 1.0).abs() < f64::EPSILON);
        assert!((vector.x - 0.6).abs() < f64::EPSILON);
        assert!((vector.y - 0.8).abs() < f64::EPSILON);
    }

    #[test]
    fn test_vector3d_dot_product() {
        let v1 = Vector3D::new(1.0, 2.0, 3.0);
        let v2 = Vector3D::new(4.0, 5.0, 6.0);
        assert_eq!(v1.dot(&v2), 32.0); // 1*4 + 2*5 + 3*6 = 32
    }

    #[test]
    fn test_vector3d_cross_product() {
        let v1 = Vector3D::new(1.0, 0.0, 0.0);
        let v2 = Vector3D::new(0.0, 1.0, 0.0);
        let cross = v1.cross(&v2);
        assert_eq!(cross.x, 0.0);
        assert_eq!(cross.y, 0.0);
        assert_eq!(cross.z, 1.0);
    }

    #[test]
    fn test_triangle_new() {
        let v0 = Point3D::new(0.0, 0.0, 0.0);
        let v1 = Point3D::new(1.0, 0.0, 0.0);
        let v2 = Point3D::new(0.0, 1.0, 0.0);
        let triangle = Triangle::new(v0, v1, v2);
        assert_eq!(triangle.v0, v0);
        assert_eq!(triangle.v1, v1);
        assert_eq!(triangle.v2, v2);
    }

    #[test]
    fn test_triangle_area() {
        let v0 = Point3D::new(0.0, 0.0, 0.0);
        let v1 = Point3D::new(2.0, 0.0, 0.0);
        let v2 = Point3D::new(0.0, 2.0, 0.0);
        let triangle = Triangle::new(v0, v1, v2);
        assert_eq!(triangle.area(), 2.0); // Right triangle with legs of length 2
    }

    #[test]
    fn test_triangle_normal() {
        let v0 = Point3D::new(0.0, 0.0, 0.0);
        let v1 = Point3D::new(1.0, 0.0, 0.0);
        let v2 = Point3D::new(0.0, 1.0, 0.0);
        let triangle = Triangle::new(v0, v1, v2);
        let normal = triangle.normal();
        
        // Normal should point in +Z direction for this triangle
        assert!((normal.x - 0.0).abs() < f64::EPSILON);
        assert!((normal.y - 0.0).abs() < f64::EPSILON);
        assert!((normal.z - 1.0).abs() < f64::EPSILON);
    }

    #[test]
    fn test_mesh_new() {
        let mesh = Mesh::new();
        assert_eq!(mesh.vertex_count(), 0);
        assert_eq!(mesh.triangle_count(), 0);
    }

    #[test]
    fn test_mesh_add_vertex() {
        let mut mesh = Mesh::new();
        let vertex = Point3D::new(1.0, 2.0, 3.0);
        mesh.add_vertex(vertex);
        assert_eq!(mesh.vertex_count(), 1);
        assert_eq!(mesh.vertices[0], vertex);
    }

    #[test]
    fn test_mesh_add_triangle() {
        let mut mesh = Mesh::new();
        let triangle = Triangle::new(
            Point3D::new(0.0, 0.0, 0.0),
            Point3D::new(1.0, 0.0, 0.0),
            Point3D::new(0.0, 1.0, 0.0),
        );
        mesh.add_triangle(triangle.clone());
        assert_eq!(mesh.triangle_count(), 1);
        assert_eq!(mesh.triangles[0].v0, triangle.v0);
    }

    #[test]
    fn test_mesh_create_cube() {
        let cube = Mesh::create_cube(2.0);
        assert_eq!(cube.vertex_count(), 8); // Cube has 8 vertices
        assert_eq!(cube.triangle_count(), 12); // Cube has 12 triangles (2 per face * 6 faces)
        
        // Verify cube vertices are at correct positions
        let half = 1.0; // size / 2
        let expected_vertices = [
            Point3D::new(-half, -half, -half),
            Point3D::new( half, -half, -half),
            Point3D::new( half,  half, -half),
            Point3D::new(-half,  half, -half),
            Point3D::new(-half, -half,  half),
            Point3D::new( half, -half,  half),
            Point3D::new( half,  half,  half),
            Point3D::new(-half,  half,  half),
        ];
        
        for (i, expected) in expected_vertices.iter().enumerate() {
            assert_eq!(cube.vertices[i], *expected);
        }
    }

    #[test]
    fn test_mesh_create_cube_different_size() {
        let cube = Mesh::create_cube(4.0);
        let half = 2.0;
        
        // Check that vertices are at the correct distance from center
        for vertex in &cube.vertices {
            assert!((vertex.x.abs() - half).abs() < f64::EPSILON);
            assert!((vertex.y.abs() - half).abs() < f64::EPSILON);
            assert!((vertex.z.abs() - half).abs() < f64::EPSILON);
        }
    }

    #[test]
    fn test_point3d_equality() {
        let p1 = Point3D::new(1.0, 2.0, 3.0);
        let p2 = Point3D::new(1.0, 2.0, 3.0);
        let p3 = Point3D::new(1.0, 2.0, 4.0);
        
        assert_eq!(p1, p2);
        assert_ne!(p1, p3);
    }

    #[test]
    fn test_vector3d_zero() {
        let zero = Vector3D::zero();
        assert_eq!(zero.x, 0.0);
        assert_eq!(zero.y, 0.0);
        assert_eq!(zero.z, 0.0);
        assert_eq!(zero.magnitude(), 0.0);
    }

    #[test]
    fn test_normalize_zero_vector() {
        let mut zero = Vector3D::zero();
        zero.normalize();
        // Normalizing zero vector should not crash and should remain zero
        assert_eq!(zero.x, 0.0);
        assert_eq!(zero.y, 0.0);
        assert_eq!(zero.z, 0.0);
    }
}