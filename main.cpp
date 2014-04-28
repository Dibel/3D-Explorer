#include "view.h"
#include <QtGui/QGuiApplication>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    View view;
    view.resize(800, 600);
    view.show();

    return app.exec();
}
