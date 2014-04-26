#include "view.h"
#include "meshobject.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <QtOpenGL/QGLFramebufferObject>
#include <Qt3D/QGLCamera>
#include <Qt3D/QGLFramebufferObjectSurface>
#include <QDesktopServices>
#include <QtCore/QDebug>

enum { Shelf, Box };

View::View() {
    /* shelf */
    MeshObject *shelf = new MeshObject(QGLAbstractScene::loadScene(":/model/shelf.obj"), MeshObject::Static);
    shelf->setPosition(QVector3D(0, 0, 0));
    shelf->setObjectId(-1);

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

    QStringList folderEntryList = dir.entryList(QDir::NoDotAndDotDot|QDir::AllDirs);
    QStringList fileEntryList = dir.entryList(QDir::Files);

    stream >> shelfSlotNum;
    qDebug() << shelfSlotNum;
    for (int i = 0; i < shelfSlotNum; ++i) {
        stream >> x >> y >> z;
        QGLBuilder builder;
        builder.newSection(QGL::Faceted);
        builder << QGLCube(6);
        builder.currentNode()->setY(3);

        MeshObject *box;

        /* +2 to skip "." and ".." */
        //if (i + 2 < entryList.size()) {
        if (i < folderEntryList.size()) {
            box = new MeshObject(builder.finalizedSceneNode(), MeshObject::Pickable);
            box->setObjectName(folderEntryList[i]);
            //box->setPath("file:///" + dir.absoluteFilePath(entryList[i + 2]));
        } else if (i< folderEntryList.size() + fileEntryList.size()) {
            box = new MeshObject(builder.finalizedSceneNode(), MeshObject::Pickable);
            box->setObjectName(fileEntryList[i - folderEntryList.size()]);
            box->setPath("file:///" + dir.absoluteFilePath(fileEntryList[i - folderEntryList.size()]));
        } else {
            box = new MeshObject(builder.finalizedSceneNode(), MeshObject::Anchor);
        }

        box->setMaterial(boxMaterial);
        box->setPosition(QVector3D(x, y, z));
        box->setObjectId(i);
		connect(box,SIGNAL(hoverChanged(bool)),this,SLOT(showFileName(bool)));
        objects.push_back(box);
    }

    camera()->setCenter(QVector3D(0, 50, 0));
    camera()->setEye(QVector3D(0, 50, 300));

    setOption(QGLView::ObjectPicking, true);
}

void View::initializeGL(QGLPainter *painter) {
    foreach(MeshObject *obj, objects)
        obj->initialize(this, painter);
}

void View::paintGL(QGLPainter *painter) {
    foreach(MeshObject *obj, objects)
		obj->draw(painter);
}

void View::showFileName(bool hovering) {
    if(hovering && !sender()->objectName().isEmpty()) {
        qDebug()<<sender()->objectName();
        //float textX=((this->camera()->projectionMatrix(4.0/3.0)*this->camera()->modelViewMatrix()*sender()->position()).x()+1)*this->width()/2;
        //float textY=(1-(this->camera()->projectionMatrix(4.0/3.0)*this->camera()->modelViewMatrix()*sender()->position()).y())*this->height()/2;
        //painter.drawText(400,300,sender()->objectName());
    } else {

    }
}

void View::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) {
        setOption(QGLView::ShowPicking, !(options() & QGLView::ShowPicking));
        update();
    }
    QGLView::keyPressEvent(event);
}

void View::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint p = event->pos();
        MeshObject *obj = qobject_cast<MeshObject*>(objectForPoint(p));
        if (obj) {
            qDebug() << obj->objectName();
            if(!obj->path().isEmpty()) {
                if(QDesktopServices::openUrl(obj->path())) {

                } else {
                    qDebug() << "Open File Failed";
                }
            }
            return;
        }
    }

	QGLView::mousePressEvent(event);
}

void View::wheelEvent(QWheelEvent *event) {
}
