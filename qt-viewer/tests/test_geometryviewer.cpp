/*
 * Theory of Operation:
 * This test suite validates the GeometryViewer OpenGL widget functionality.
 * It tests widget initialization, geometry collection integration, mouse interactions,
 * view operations (reset, fit-to-window), and basic rendering capabilities.
 * The tests create geometry collections with points and lines, simulate user
 * interactions, and verify proper signal emissions and visual rendering.
 */

#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QMouseEvent>
#include "../geometryviewer.h"
#include "../../stdgeo_lib.h"

// Test class for GeometryViewer OpenGL widget validation
class TestGeometryViewer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testInitialization();
    void testGeometryCollection();
    void testMouseInteraction();
    void testViewOperations();
    void testRendering();

private:
    GeometryViewer *m_viewer;
    GeometryCollection *m_collection;
    QApplication *m_app;
};

// Sets up Qt application instance for OpenGL testing
void TestGeometryViewer::initTestCase()
{
    // Initialize Qt application if not already done
    if (!QApplication::instance()) {
        static int argc = 1;
        static const char* argv[] = {"test", nullptr};
        m_app = new QApplication(argc, const_cast<char**>(argv));
    }
}

// Cleans up after all tests complete
void TestGeometryViewer::cleanupTestCase()
{
    // Cleanup is handled by Qt
}

// Creates geometry collection and viewer instance for each test
void TestGeometryViewer::init()
{
    m_collection = geometry_collection_new();
    m_viewer = new GeometryViewer();
    m_viewer->setGeometryCollection(m_collection);
    m_viewer->show();
    
    // Wait for the widget to be visible
    [[maybe_unused]] bool exposed = QTest::qWaitForWindowExposed(m_viewer);
}

// Cleans up viewer and geometry collection after each test
void TestGeometryViewer::cleanup()
{
    delete m_viewer;
    geometry_collection_free(m_collection);
}

// Verifies proper OpenGL widget initialization and visibility
void TestGeometryViewer::testInitialization()
{
    QVERIFY(m_viewer != nullptr);
    QVERIFY(m_viewer->isVisible());
    QCOMPARE(m_viewer->width() > 0, true);
    QCOMPARE(m_viewer->height() > 0, true);
}

// Tests geometry collection operations and data integrity
void TestGeometryViewer::testGeometryCollection()
{
    // Test with empty collection
    QCOMPARE(geometry_collection_size(m_collection), 0);
    
    // Add a point
    int pointIndex = geometry_collection_add_point(m_collection, 1.0, 2.0);
    QVERIFY(pointIndex >= 0);
    QCOMPARE(geometry_collection_size(m_collection), 1);
    
    // Add a line
    int lineIndex = geometry_collection_add_line(m_collection, 0.0, 0.0, 3.0, 4.0);
    QVERIFY(lineIndex >= 0);
    QCOMPARE(geometry_collection_size(m_collection), 2);
    
    // Verify geometry retrieval
    CGeometry geometry;
    QCOMPARE(geometry_collection_get(m_collection, pointIndex, &geometry), 0);
    QCOMPARE(geometry.geometry_type, GEOMETRY_TYPE_POINT);
    QCOMPARE(geometry.data.point.x, 1.0);
    QCOMPARE(geometry.data.point.y, 2.0);
    
    QCOMPARE(geometry_collection_get(m_collection, lineIndex, &geometry), 0);
    QCOMPARE(geometry.geometry_type, GEOMETRY_TYPE_LINE);
    QCOMPARE(geometry.data.line.start.x, 0.0);
    QCOMPARE(geometry.data.line.start.y, 0.0);
    QCOMPARE(geometry.data.line.end.x, 3.0);
    QCOMPARE(geometry.data.line.end.y, 4.0);
}

// Validates mouse position tracking and coordinate transformation
void TestGeometryViewer::testMouseInteraction()
{
    // Add some geometry to make interactions meaningful
    geometry_collection_add_point(m_collection, 0.0, 0.0);
    geometry_collection_add_line(m_collection, -1.0, -1.0, 1.0, 1.0);
    
    // Test mouse position signal
    QSignalSpy positionSpy(m_viewer, &GeometryViewer::mousePositionChanged);
    
    // Simulate mouse move
    QPoint testPoint(m_viewer->width() / 2, m_viewer->height() / 2);
    QMouseEvent moveEvent(QEvent::MouseMove, testPoint, m_viewer->mapToGlobal(testPoint), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_viewer, &moveEvent);
    
    // Check that signal was emitted
    QVERIFY(positionSpy.count() > 0);
    
    // Verify signal arguments (coordinates should be reasonable)
    QList<QVariant> arguments = positionSpy.takeFirst();
    double x = arguments.at(0).toDouble();
    double y = arguments.at(1).toDouble();
    
    // Coordinates should be within reasonable bounds
    QVERIFY(qAbs(x) < 1000.0);
    QVERIFY(qAbs(y) < 1000.0);
}

// Tests view control functions like reset and fit-to-window
void TestGeometryViewer::testViewOperations()
{
    // Add some geometry
    geometry_collection_add_point(m_collection, 5.0, 5.0);
    geometry_collection_add_point(m_collection, -5.0, -5.0);
    
    // Test reset view
    m_viewer->resetView();
    // No direct way to verify, but should not crash
    
    // Test fit to window
    m_viewer->fitToWindow();
    // Should adjust view to show all geometry
    
    // Wait for any pending updates
    QTest::qWait(100);
}

// Validates OpenGL rendering and visual output generation
void TestGeometryViewer::testRendering()
{
    // Add various geometry types
    geometry_collection_add_point(m_collection, 1.0, 1.0);
    geometry_collection_add_point(m_collection, -1.0, -1.0);
    geometry_collection_add_line(m_collection, -2.0, 0.0, 2.0, 0.0);
    geometry_collection_add_line(m_collection, 0.0, -2.0, 0.0, 2.0);
    
    // Force a repaint
    m_viewer->update();
    QTest::qWait(100);
    
    // Grab the rendered content
    QPixmap pixmap = m_viewer->grab();
    QVERIFY(!pixmap.isNull());
    QVERIFY(pixmap.width() > 0);
    QVERIFY(pixmap.height() > 0);
    
    // Basic check - image should not be completely uniform
    QImage image = pixmap.toImage();
    QRgb firstPixel = image.pixel(0, 0);
    bool hasVariation = false;
    
    // Sample a few pixels to see if there's any variation
    for (int i = 0; i < qMin(100, image.width() * image.height()); i += 10) {
        int x = (i % image.width());
        int y = (i / image.width());
        if (y < image.height() && image.pixel(x, y) != firstPixel) {
            hasVariation = true;
            break;
        }
    }
    
    // Should have some variation (not just a solid color)
    QVERIFY(hasVariation);
}

QTEST_MAIN(TestGeometryViewer)
#include "test_geometryviewer.moc"

