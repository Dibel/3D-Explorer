#include "view.h"
#include "window.h"
#include <QtGui/QGuiApplication>
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    View view;
    view.resize(800, 600);
    view.show();
    //QWidget *widget = QWidget::createWindowContainer(&view);
    //widget->resize(800, 600);

    //Window window(widget);

    //widget->show();

    //Window window;
    //window.show();

    return app.exec();
}
