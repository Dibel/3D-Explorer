#include "view.h"
#include <QtGui/QGuiApplication>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    View scene;
    scene.resize(800, 600);
    scene.show();

    return app.exec();
}
