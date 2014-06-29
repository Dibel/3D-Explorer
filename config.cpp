#include "common.h"
#include "room.h"
#include <Qt3D/QGLAbstractScene>
#include <QtGui/QImage>
#include <QtCore/QFile>

#include <QtCore/QDebug>

int roomWidth, roomLength, roomHeight, eyeHeight;
qreal boxScale;

QHash<QString, QGLMaterial*> palette;
QHash<QString, QGLSceneNode*> models;
QHash<QString, Room*> rooms;

QList<QStringList> typeFilters;
QStringList typeNameList;
QHash<QString, int> extToIndex;

void loadConfig(const QString &fileName);
void loadProperty(const QString &property, QTextStream &value);

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
        rooms.insert(name, new Room(name + ".conf"));

    } else if (property == "model") {
        QString name, fileName;
        value >> name >> fileName;
        QGLAbstractScene *model = QGLAbstractScene::loadScene(dataDir + fileName);
        models.insert(name, model->mainNode());

    } else if (property == "filetype") {
        QString type, ext;
        QStringList filter;

        value >> type >> ext;
        while (!ext.isEmpty()) {
            filter.append("*." + ext);
            extToIndex[ext] = typeNameList.size() + 2;
            value >> ext;
        }

        typeFilters.append(filter);
        typeNameList.append(type);

    } else
        qDebug() << "Unknown property" << property;
}
