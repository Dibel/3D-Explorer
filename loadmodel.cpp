#include "view.h"
#include "meshobject.h"
#include "common.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLMaterialCollection>
#include <QSharedPointer>

int roomLength = 160;
int roomWidth = 240;
int roomHeight = 120;
int eyeHeight = 50;
qreal boxScale = 0.025;

static void FixNodesRecursive(int matIndex, QGLSceneNode* pNode)
{
    pNode->setMaterialIndex(matIndex);
    pNode->setEffect(QGL::LitModulateTexture2D);
    for (auto node : pNode->allChildren()) {
        node->setMaterialIndex(matIndex);
        node->setEffect(QGL::LitModulateTexture2D);
    };
}


void View::loadConfig(const QString &fileName) {
    QFile file(configDir + fileName);
    file.open(QIODevice::ReadOnly);
    QString line, property;
    QTextStream stream;
    stream.setString(&line, QIODevice::ReadOnly);

    while (!file.atEnd()) {
        line = file.readLine();
        if (line.isEmpty() || line[0] == '#' || line[0] == '\n') continue;
        if (line[0] == '[') {
            property = line.mid(1, line.indexOf(']') - 1);
        } else {
            stream.seek(0);
            loadProperty(property, stream);
        }
    }

    file.close();
}

void View::loadProperty(const QString &property, QTextStream &value) {
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
        QString fileName; value >> fileName;
        curRoom = new Room(fileName, palette, this);

    } else if (property == "model") {
        //loadModel(value);

    } else
        qDebug() << "Unknown property" << property;
}

void View::loadModels() {
    QGLAbstractScene *model;
    MeshObject *mesh;

    loadConfig("main.conf");

    mat2 = palette["tmp2"];

    /* arrows */
    model = QGLAbstractScene::loadScene(dataDir + QString("leftarrow.obj"));
    model->setParent(this);
    model->mainNode()->setMaterial(palette["tmp2"]);
    mesh = new MeshObject(model->mainNode(), this, LeftArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(-50, 90, -roomLength / 2));
    curRoom->solid << mesh;

    model = QGLAbstractScene::loadScene(dataDir + QString("rightarrow.obj"));
    model->setParent(this);
    model->mainNode()->setMaterial(palette["tmp2"]);
    mesh = new MeshObject(model->mainNode(), this, RightArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(50, 90, -roomLength / 2));
    curRoom->solid << mesh;
}

//void View::loadModel(QTextStream &value) {
//    QString name, mat;
//    qreal tmp;
//    int anim, recursive;
//    value >> name >> anim >> mat >> recursive;
//    qDebug() << "loaded model" << name << anim << mat << recursive;
//
//    QGLAbstractScene *model = QGLAbstractScene::loadScene(dataDir + name + ".obj");
//    model->setParent(this);
//
//    //if (recursive) {
//    //    int matIndex = model->mainNode()->palette()->addMaterial(palette[mat]);
//    //    FixNodesRecursive(matIndex, model->mainNode());
//    //} else
//    //    model->mainNode()->setMaterial(palette[mat]);
//
//    MeshObject *mesh = new MeshObject(model->mainNode(), this, -2);
//
//    if (anim) {
//        qreal x, y, z;
//        value >> x >> y >> z;
//        mesh->setAnimVector(x, y, z);
//        value >> x >> y >> z;
//        mesh->setAnimCenter(x, y, z);
//
//        model = QGLAbstractScene::loadScene(dataDir + name + "_anim.obj");
//        model->setParent(this);
//        //if (recursive) {
//        //    int matIndex = model->mainNode()->palette()->addMaterial(palette[mat]);
//        //    FixNodesRecursive(matIndex, model->mainNode());
//        //} else
//        //    model->mainNode()->setMaterial(palette[mat]);
//        mesh->setAnimMesh(model->mainNode());
//    }
//
//    models.insert(name, mesh);
//}
