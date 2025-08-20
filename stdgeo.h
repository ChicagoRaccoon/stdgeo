#ifndef STDGEO_LIB_H
#define STDGEO_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double x;
    double y;
} CPoint;

typedef struct {
    CPoint start;
    CPoint end;
} CLine;

typedef enum {
    GEOMETRY_TYPE_POINT = 0,
    GEOMETRY_TYPE_LINE = 1
} CGeometryType;

typedef union {
    CPoint point;
    CLine line;
} CGeometryData;

typedef struct {
    CGeometryType geometry_type;
    CGeometryData data;
} CGeometry;

// Opaque handle to geometry collection
typedef struct GeometryCollection GeometryCollection;

// Collection management
GeometryCollection* geometry_collection_new(void);
void geometry_collection_free(GeometryCollection* collection);
int geometry_collection_size(const GeometryCollection* collection);
int geometry_collection_clear(GeometryCollection* collection);

// Adding geometries
int geometry_collection_add_point(GeometryCollection* collection, double x, double y);
int geometry_collection_add_line(GeometryCollection* collection, 
                                double x1, double y1, double x2, double y2);

// Accessing geometries
int geometry_collection_get(const GeometryCollection* collection, int index, CGeometry* out_geometry);
int geometry_collection_get_all(const GeometryCollection* collection, 
                               const CGeometry** out_geometries, int* out_count);

// Transformations
int geometry_collection_translate(GeometryCollection* collection, double dx, double dy);
int geometry_collection_rotate(GeometryCollection* collection, double angle, 
                              double center_x, double center_y);

// File I/O
int geometry_collection_load_json(GeometryCollection* collection, const char* filename);
int geometry_collection_save_json(const GeometryCollection* collection, const char* filename);

// Utility functions
int geometry_collection_bounding_box(const GeometryCollection* collection,
                                   double* min_x, double* min_y, 
                                   double* max_x, double* max_y);

#ifdef __cplusplus
}
#endif

#endif // STDGEO_LIB_H