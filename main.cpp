#include "view.h"
#include <QtGui/QGuiApplication>
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    View view(800,600);
    view.show();
    return app.exec();
}
