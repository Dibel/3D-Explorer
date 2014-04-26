#include "view.h"
#include "mainwindow.h"
#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    //view.resize(800, 600);
    //view.setMaximumSize(QSize(800,600));
    //view.setMinimumSize(QSize(800,600));
    //view.show();
    return app.exec();
}
