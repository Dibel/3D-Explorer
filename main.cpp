#include "common.h"
#include "room.h"
#include "view.h"
#include <QtWidgets/QApplication>
#include <QtCore/QFile>
#include <Qt3D/QGLTexture2D>

GLView *view;

int roomWidth, roomLength, roomHeight, eyeHeight;
qreal boxScale;
QHash<QString, QGLMaterial*> palette;
QHash<QString, QSet<QString>> typeFilter;
QHash<QString, Room*> rooms;

void loadConfig(const QString &fileName);
void loadProperty(const QString &property, QTextStream &value);

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    View *view_ = new View(800, 600);
    view = view_;
    loadConfig("main.conf");
    view_->load();
    view_->show();

    return app.exec();
}

void loadConfig(const QString &fileName)
{
    QFile file(configDir + fileName);
    file.open(QIODevice::ReadOnly);
    QString line, property;
    QTextStream stream;
    stream.setString(&line, QIODevice::ReadOnly);

    while (!file.atEnd()) {
        line = file.readLine();
        if (line.isEmpty() || line[0] == '#' || line[0] == '\n' || (line[0] == '\r' && line[1] == '\n')) continue;
        if (line[0] == '[') {
            property = line.mid(1, line.indexOf(']') - 1);
        } else {
            stream.seek(0);
            loadProperty(property, stream);
        }
    }

    file.close();
}

void loadProperty(const QString &property, QTextStream &value) {
    if (property == "material") {
        QGLMaterial *mat = new QGLMaterial();
        QString name, file;
        value >> name >> file;

        if (file != "-") {
            QGLTexture2D *tex = new QGLTexture2D();
            tex->setImage(QImage(dataDir + file));
            mat->setTexture(tex);
            mat->setTextureCombineMode(QGLMaterial::Decal);
        }

        int r, g, b;
        value >> r >> g >> b; if (r != -1) mat->setAmbientColor(QColor(r, g, b));
        value >> r >> g >> b; if (r != -1) mat->setDiffuseColor(QColor(r, g, b));
        value >> r >> g >> b; if (r != -1) mat->setSpecularColor(QColor(r, g, b));
        qreal shin; value >> shin; mat->setShininess(shin);

        palette.insert(name, mat);

    } else if (property == "size") {
        value >> roomWidth >> roomLength >> roomHeight >> eyeHeight >> boxScale;

    } else if (property == "room") {
        QString name; value >> name;
        rooms.insert(name, new Room(name, 0, 0));

    } else if (property == "model") {
        //loadModel(value);

    } else if (property == "filetype") {
        QString name, tmp;
        QSet<QString> filter;

        value >> name >> tmp;
        while (!tmp.isEmpty()) {
            filter.insert("*." + tmp);
            value >> tmp;
        }

        typeFilter.insert(name, filter);

    } else
        qDebug() << "Unknown property" << property;
}
