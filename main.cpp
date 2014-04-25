#include "view.h"
#include <QtGui/QGuiApplication>
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    View view;
    view.resize(800, 600);
    view.setMaximumSize(QSize(800,600));
    view.setMinimumSize(QSize(800,600));
    view.show();

    return app.exec();
}
