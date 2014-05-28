#include "view.h"
#include "meshobject.h"
#include "common.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLMaterialCollection>
#include <QSharedPointer>

void View::loadModels() {
    QGLAbstractScene *model;
    MeshObject *mesh;
    int matIndex;

    QFile file(":/config/main.conf");
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    QString name = stream.readLine();
    QString extra;

    QFile file2;
    QTextStream stream2;

    static int id[4] = { -1, -1, TrashBin, Door };

    QMatrix4x4 doorTrans;
    doorTrans.translate(QVector3D(21, 0, 0));

    QGLTexture2D *tex;

    while (name != "[material]" && !stream.atEnd()) name = stream.readLine();
    while (true) {
        stream >> name;
        if (name[0] == '[') { stream.readLine(); break; }
        if (name[0] == '#') { stream.readLine(); continue; }

        QGLMaterial *m = new QGLMaterial();

        stream >> extra;

        if (extra != "-") {
            tex = new QGLTexture2D();
            tex->setImage(QImage(":/maps/" + extra));
            m->setTexture(tex);
	    m->setTextureCombineMode(QGLMaterial::Decal);
        }

        int r, g, b;
        stream >> r >> g >> b;
        if (r != -1) m->setAmbientColor(QColor(r, g, b));
        stream >> r >> g >> b;
        if (r != -1) m->setDiffuseColor(QColor(r, g, b));
        stream >> r >> g >> b;
        if (r != -1) m->setSpecularColor(QColor(r, g, b));
        stream >> r; m->setShininess(r);

        //palette.push_back(m);
        palette.insert(name, m);
        stream.readLine();
    }

    mat2 = palette["tmp2"];
    qDebug() << palette;

    while (name != "[model]" && !stream.atEnd()) name = stream.readLine();


    curRoom = new Room("room1.conf", palette, this);

    file.close();

    /* arrows */
    model = QGLAbstractScene::loadScene(":/model/leftarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(palette["tmp2"]);
    mesh = new MeshObject(model, this, LeftArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(-50, 90, -roomLength / 2));
    curRoom->solid << mesh;

    model = QGLAbstractScene::loadScene(":/model/rightarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(palette["tmp2"]);
    mesh = new MeshObject(model, this, RightArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(50, 90, -roomLength / 2));
    curRoom->solid << mesh;

}
