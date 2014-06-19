#include "common.h"
#include "room.h"
#include "view.h"
#include <QtWidgets/QApplication>
#include <QtCore/QFile>
#include <Qt3D/QGLTexture2D>

#include <QtCore/QDebug>

void loadConfig(const QString &fileName);

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    View *view_ = new View(800, 600);
    view = view_;
    loadConfig("main.conf");
    view_->load();
    view_->show();

    return app.exec();
}
