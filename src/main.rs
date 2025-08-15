use stdgeo::geo::*;

fn main() {
    println!("StdGeo 3D Geometry Library Test");
    
    // Test basic geometry types
    let p1 = Point3D::new(0.0, 0.0, 0.0);
    let p2 = Point3D::new(1.0, 1.0, 1.0);
    println!("Point 1: {}", p1);
    println!("Point 2: {}", p2);
    println!("Distance: {:.3}", p1.distance_to(&p2));
    
    let mut v1 = Vector3D::new(3.0, 4.0, 0.0);
    println!("Vector: {} (magnitude: {:.3})", v1, v1.magnitude());
    v1.normalize();
    println!("Normalized: {} (magnitude: {:.3})", v1, v1.magnitude());
    
    // Test mesh creation
    let cube = Mesh::create_cube(2.0);
    println!("{}", cube);
    
    println!("Rust geometry library working!");
}