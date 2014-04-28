#include "view.h"
#include "meshobject.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <QtOpenGL/QGLFramebufferObject>
#include <Qt3D/QGLCamera>
#include <Qt3D/QGLFramebufferObjectSurface>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QtWidgets/QWidget>
#include <Qt3D/QGLShaderProgramEffect>

#include <QtCore/QDebug>

enum { Shelf, Box };

View::View() : pickedObj(NULL), enteredObject(NULL), hudObj(new QGLSceneNode(this)), hudEffect(NULL) {
    camera()->setCenter(QVector3D(0, 50, 0));
    camera()->setEye(QVector3D(0, 50, 300));

    qreal aspectRatio = 1.0 * width() / height();
    mvp = camera()->projectionMatrix(aspectRatio) * camera()->modelViewMatrix();

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
    initializeBox();

    /* HUD */
    QGLBuilder builder;
    QGLSceneNode *root = builder.sceneNode();

    QGLMaterial *mat = new QGLMaterial;

    QGLTexture2D *tex = new QGLTexture2D;
    //QImage image("tex.png");
    QImage image = paintHud();
    tex->setImage(image);
    tex->bind();
    mat->setTexture(tex);

    int hudMat = root->palette()->addMaterial(mat);

    builder.pushNode()->setObjectName("HUD");
    builder.addPane(QSizeF(2, 2));
    builder.currentNode()->setMaterialIndex(hudMat);
    
    hudEffect = new QGLShaderProgramEffect();
    hudEffect->setVertexShaderFromFile(":/hud.vsh");
    hudEffect->setFragmentShaderFromFile(":/hud.fsh");
    builder.currentNode()->setUserEffect(hudEffect);

    hudObj = builder.finalizedSceneNode();
}

QImage View::paintHud() {
    QImage ret(800, 600, QImage::Format_ARGB32_Premultiplied);
    ret.fill(Qt::transparent);
    QPainter painter(&ret);
    painter.setPen(QColor(Qt::red));
    painter.drawText(400, 300, "Hello World!!!");
    return ret;
}

void View::resizeEvent(QResizeEvent *e) {
}

void View::initializeGL(QGLPainter *painter) {
    foreach(MeshObject *obj, objects)
        obj->initialize(this, painter);
}

void View::paintGL(QGLPainter *painter) {
    foreach(MeshObject *obj, objects)
        obj->draw(painter);

    if (!painter->isPicking()) {
        glEnable(GL_BLEND);

        hudEffect->setActive(painter, true);
        hudObj->draw(painter);

        glDisable(GL_BLEND);
    }
}

void View::initializeBox() {
    QFile file(":/model/shelf.slots");
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);

    qreal x, y, z;

    QGLMaterial *boxMaterial = new QGLMaterial();
    boxMaterial->setAmbientColor(QColor(255, 255, 255));
    boxMaterial->setDiffuseColor(QColor(150, 150, 150));
    boxMaterial->setSpecularColor(QColor(255, 255, 255));
    boxMaterial->setShininess(128);

    QStringList folderEntryList = dir.entryList(QDir::NoDot|QDir::AllDirs);
    QStringList fileEntryList = dir.entryList(QDir::Files);

    stream >> shelfSlotNum;
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

void View::mouseDoubleClickEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        if(pickedObj) {
            qDebug() << pickedObj->objectName();
            if(!pickedObj->path().isEmpty()) {
                if(QDesktopServices::openUrl(pickedObj->path())) {

                } else {
                    qDebug() << "Open File Failed";
                }
            } else {
                enteredObject = NULL;
                dir.cd(pickedObj->objectName());
                for(int i=objects.size()-1;i>0;--i) {
                    disconnect(objects.at(i),SIGNAL(hoverChanged(bool)),this,SLOT(showFileName(bool)));
                    deregisterObject(i-1);
                    delete objects.at(i);
                }
                objects.erase(objects.begin()+1,objects.end());
                initializeBox();
                for(int i=objects.size()-1;i>0;--i) {
                    registerObject(i-1,objects.at(i));
                }
                update();
            }
            pickedObj=NULL;
        } else {
            QPoint p=event->pos();
            pickedObj = qobject_cast<MeshObject*>(objectForPoint(event->pos()));
            if (pickedObj && pickedObj->pickType() == MeshObject::Pickable) {
                qDebug() << pickedObj->objectName();
                if(!pickedObj->path().isEmpty()) {
                    if(QDesktopServices::openUrl(pickedObj->path())) {

                    } else {
                        qDebug() << "Open File Failed";
                    }
                }
            }
            pickedObj = NULL;
        }
    }
}

void View::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        pickedObj = qobject_cast<MeshObject*>(objectForPoint(event->pos()));
        if (pickedObj && pickedObj->pickType() == MeshObject::Pickable) {
            pickedObj->setPickType(MeshObject::Picked);

            qreal aspectRatio = static_cast<qreal>(width()) / height();
            mvp = camera()->projectionMatrix(aspectRatio) * camera()->modelViewMatrix();

            pickedPos = pickedObj->position();
            QVector3D pickedPos_screen = mvp * pickedPos;
            pickedDepth = pickedPos_screen.z();

            qreal x = event->pos().x() * 2.0 / width() - 1;
            qreal y = 1 - event->pos().y() * 2.0 / height();
            QVector3D clickedPos_screen(x, y, pickedDepth);
            QVector3D clickedPos_world = mvp.inverted() * clickedPos_screen;
            pickedModelPos = clickedPos_world - pickedPos;

            update();
            return;
        } else
            pickedObj = NULL;
    }

    QGLView::mousePressEvent(event);
}

void View::wheelEvent(QWheelEvent *event) {
}

void View::mouseReleaseEvent(QMouseEvent *event) {
    if (pickedObj && event->button() == Qt::LeftButton) {
        MeshObject *anchor = qobject_cast<MeshObject*>(objectForPoint(event->pos()));
        if (anchor) {
            QVector3D destPos = anchor->position();
            anchor->setPosition(pickedPos);
            pickedObj->setPosition(destPos);
        } else {
            pickedObj->setPosition(pickedPos);
        }

        pickedObj->setPickType(MeshObject::Pickable);
        pickedObj = NULL;

        update();
        return;
    }

    QGLView::mouseReleaseEvent(event);
}

void View::mouseMoveEvent(QMouseEvent *event) {
    if (pickedObj) {
        qreal x = event->pos().x() * 2.0 / width() - 1;
        qreal y = 1 - event->pos().y() * 2.0 / height();
        QVector3D screenPos(x, y, pickedDepth);
        QVector3D worldPos = mvp.inverted() * screenPos;
        pickedObj->setPosition(worldPos - pickedModelPos);

        update();
        return;
    }
    QObject *object = objectForPoint(event->pos());
    if (object) {
        if (object != enteredObject) {
            if (enteredObject)
                sendLeaveEvent(enteredObject);
            enteredObject = object;
            sendEnterEvent(enteredObject);
        }
        QMouseEvent e
            (QEvent::MouseMove, QPoint(0, 0),
             event->globalPos(), event->button(), event->buttons(), event->modifiers());
        QCoreApplication::sendEvent(object, &e);
    } else if (enteredObject) {
        sendLeaveEvent(enteredObject);
        enteredObject = NULL;
    } else {
        QGLView::mouseMoveEvent(event);
    }
}


void View::sendEnterEvent(QObject *object)
{
    QEvent event(QEvent::Enter);
    QCoreApplication::sendEvent(object, &event);
}

void View::sendLeaveEvent(QObject *object)
{
    QEvent event(QEvent::Leave);
    QCoreApplication::sendEvent(object, &event);
}
