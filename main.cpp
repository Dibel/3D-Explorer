#include "common.h"
#include "view.h"
#include <QtWidgets/QApplication>
#include <Qt3D/QGLTexture2D>

void loadConfig(const QString &fileName);

int windowWidth, windowHeight;

int hoveringId = -1;
QColor hoveringPickColor;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    loadConfig("main.conf");

    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setDepthBufferSize(24);
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    format.setAlphaBufferSize(8);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSamples(4);

    View *view = new View(800, 600, format);
    view->show();

    return app.exec();
}
