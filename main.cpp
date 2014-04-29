#include "view.h"
#include <QtGui/QGuiApplication>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    View view(800,600);
    view.show();
    return app.exec();
}
