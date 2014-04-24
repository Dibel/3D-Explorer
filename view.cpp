#include "view.h"
#include "meshobject.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>

#include <QtCore/QDebug>

enum { Shelf, Box };

View::View() {
    /* shelf */
    MeshObject *shelf = new MeshObject(QGLAbstractScene::loadScene(":/model/shelf.obj"));
    shelf->setPosition(QVector3D(0, 0, 0));
    shelf->setObjectId(Shelf);

    QGLMaterial *shelfMaterial = new QGLMaterial();
    shelfMaterial->setAmbientColor(QColor(192, 150, 128));
    shelfMaterial->setSpecularColor(QColor(60, 60, 60));
    shelfMaterial->setShininess(128);

    shelf->setMaterial(shelfMaterial);

    objects.push_back(shelf);

    /* boxes */
    QFile file(":/model/shelf.slots");
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);

    qreal x, y, z;
    QGLMaterial *boxMaterial = new QGLMaterial();
    boxMaterial->setAmbientColor(QColor(255, 255, 255));
    boxMaterial->setDiffuseColor(QColor(150, 150, 150));
    boxMaterial->setSpecularColor(QColor(255, 255, 255));
    boxMaterial->setShininess(128);

    stream >> shelfSlotNum;
    qDebug() << shelfSlotNum;
    for (int i = 0; i < shelfSlotNum; ++i) {
        stream >> x >> y >> z;
        QGLBuilder builder;
        builder.newSection(QGL::Faceted);
        builder << QGLCube(6);
        builder.currentNode()->setY(3);
        MeshObject *box = new MeshObject(builder.finalizedSceneNode());
        box->setMaterial(boxMaterial);
        box->setPosition(QVector3D(x, y, z));
        box->setObjectId(Box);
        objects.push_back(box);
    }

    camera()->setCenter(QVector3D(0, 50, 0));
    camera()->setEye(QVector3D(0, 50, 300));
}

void View::initializeGL(QGLPainter *painter) {
    foreach(MeshObject *obj, objects)
        obj->initialize(this, painter);
}

void View::paintGL(QGLPainter *painter) {
    foreach(MeshObject *obj, objects)
        obj->draw(painter);
}
