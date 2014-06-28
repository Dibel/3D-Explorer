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
    View *view = new View(800, 600);
    view->show();

    return app.exec();
}
