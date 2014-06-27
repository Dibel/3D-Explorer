#include "common.h"
#include "room.h"
#include "view.h"
#include <QtWidgets/QApplication>
#include <QtCore/QFile>
#include <Qt3D/QGLTexture2D>
#include <Qt3D/QGLAbstractScene>

#include <QtCore/QDebug>

int paintingOutline = -1;
QColor hoveringPickColor;

GLView *view;

int roomWidth, roomLength, roomHeight, eyeHeight;
qreal boxScale;

QHash<QString, QGLMaterial*> palette;
QHash<QString, QGLSceneNode*> models;
QHash<QString, Room*> rooms;
QList<QStringList> typeFilters;
QHash<QString, QString> fileType;
QStringList typeList;
QHash<QString, int> extToIndex;

void loadConfig(const QString &fileName);
void loadProperty(const QString &property, QTextStream &value);

inline void setAllMaterial(QGLSceneNode *node, QGLMaterial *mat)
{
    int index = node->palette()->addMaterial(mat);
    node->setMaterialIndex(index);
    node->setEffect(QGL::LitModulateTexture2D);
    for (QGLSceneNode *child : node->allChildren()) {
        child->setMaterialIndex(index);
        child->setEffect(QGL::LitModulateTexture2D);
    }
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
        rooms.insert(name, new Room(name));

    } else if (property == "model") {
        QString name, mat;
        value >> name >> mat;
        qDebug() << name << mat;
        QGLAbstractScene *model = QGLAbstractScene::loadScene(dataDir + name + ".obj");
        model->setParent(view);
        if (mat != "-")
            setAllMaterial(model->mainNode(), palette[mat]);
        models.insert(name, model->mainNode());

    } else if (property == "filetype") {
        QString type, ext;
        QStringList filter;

        value >> type >> ext;
        while (!ext.isEmpty()) {
            filter.append("*." + ext);
            fileType[ext] = type;
            extToIndex[ext] = typeList.size() + 2;
            value >> ext;
        }

        typeFilters.append(filter);
        typeList.append(type);

    } else
        qDebug() << "Unknown property" << property;
}
