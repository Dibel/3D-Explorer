#include "view.h"
#include "meshobject.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QDesktopServices>

#include <QtCore/QDebug>

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);
static QVector3D extendTo3D(const QPoint &pos, qreal depth);

View::View(int width = 800, int height = 600)
    : pickedObject(NULL), enteredObject(NULL), hudObj(NULL), hudEffect(NULL), picture(NULL)
{
    setWidth(width);
    setHeight(height);
    camera()->setCenter(QVector3D(0, 50, 0));
    camera()->setEye(QVector3D(0, 50, 300));

    mvp = calcMvp(camera(), size());

    /* shelf */
    background = new MeshObject(
            QGLAbstractScene::loadScene(":/model/shelf.obj"),
            MeshObject::Static);
    background->setPosition(QVector3D(0, 0, 0));
    background->setObjectId(-1);

    QGLMaterial *shelfMaterial = new QGLMaterial();
    shelfMaterial->setAmbientColor(QColor(192, 150, 128));
    shelfMaterial->setSpecularColor(QColor(60, 60, 60));
    shelfMaterial->setShininess(128);

    background->setMaterial(shelfMaterial);

    /* boxes */
    initializeBox();

    /* HUD */
    initializeHud();
    /* FIXME: show dir on start */
}

QImage View::paintHud(float x, float y, QString text) {
    QImage ret(width(), height(), QImage::Format_ARGB32_Premultiplied);
    ret.fill(Qt::transparent);
    QPainter painter(&ret);
    QFont font=painter.font();
    font.setPointSize(16);
    font.setBold(1);
    painter.setFont(font);
    painter.setPen(QColor(Qt::red));
    painter.drawText(x, y, text);
    painter.drawText(0,20,dir.absolutePath());
    return ret;
}

void View::drawText(float x, float y, QString text) {
    QGLTexture2D *tex = hudObj->material()->texture();
    /* FIXME: cleanupResources() will crash the program */
    //tex->cleanupResources();
    /* FIXME: delete tex will crash the program on Windows*/
    //if(tex != NULL)
    //    delete tex;

    tex = new QGLTexture2D();
    QImage image = paintHud(x,y,text);
    tex->setImage(image);
    tex->bind();
    tex->clearImage();

    hudObj->material()->setTexture(tex);
}

void View::initializeHud() {
    QGLBuilder builder;
    builder.sceneNode();
    builder.pushNode();
    builder.addPane(QSizeF(2, 2));

    hudEffect = new QGLShaderProgramEffect();
    hudEffect->setVertexShaderFromFile(":/hud.vsh");
    hudEffect->setFragmentShaderFromFile(":/hud.fsh");
    builder.currentNode()->setUserEffect(hudEffect);

    hudObj = builder.finalizedSceneNode();
    hudObj->setMaterial(new QGLMaterial);

}

void View::resizeEvent(QResizeEvent *e) {
    drawText(0,0,"");
    update();
}

void View::initializeGL(QGLPainter *painter) {
    foreach(MeshObject *obj, boxes)
        obj->initialize(this, painter);
}

void View::paintGL(QGLPainter *painter) {
    mvp = calcMvp(camera(), size());
    background->draw(painter);

    foreach(MeshObject *obj, boxes)
        obj->draw(painter);

    if(picture != NULL) {
        painter->modelViewMatrix().scale(1.0,0.75,1.0);
        painter->modelViewMatrix().translate(40.0,50.0,0.0);
        picture->draw(painter);
    }

    if (!painter->isPicking() && hudEffect) {
        glEnable(GL_BLEND);
        hudEffect->setActive(painter, true);
        if(hudObj != NULL) {
            hudObj->draw(painter);
        }
        glDisable(GL_BLEND);
    }

}

void View::updateBoxes() {
    /* TODO: What's this? */
    //Show picture
    QStringList filter;
    filter << "*.bmp" << "*.jpg" << "*.jpeg" << "*.gif" << "*.png";
    QStringList pictureEntryList = dir.entryList(filter, QDir::Files);
    if(!pictureEntryList.isEmpty()) {
        QGLBuilder b;
        b.addPane(50.0f);
        picture = b.finalizedSceneNode();

        QImage image(pictureEntryList.at(0));

        // put the image into a material and stick in onto the triangles
        QGLTexture2D *tex = new QGLTexture2D;
        tex->setImage(image);
        QGLMaterial *mat = new QGLMaterial;
        mat->setTexture(tex);
        picture->setMaterial(mat);
        picture->setEffect(QGL::FlatDecalTexture2D);
    }

    /* update entry info */
    QStringList entryList = dir.entryList(
            QDir::AllEntries | QDir::NoDot, QDir::DirsFirst);
    entryCnt = entryList.size();
    dirEntryCnt = dir.entryList(QDir::NoDot | QDir::Dirs).size();

    for (int i = 0; i < entryCnt && i < slotCnt; ++i) {
        boxes[i]->setPickType(MeshObject::Pickable);
        boxes[i]->setObjectName(entryList[i]);
    }
    for (int i = entryCnt; i < slotCnt; ++i) {
        boxes[i]->setPickType(MeshObject::Anchor);
        boxes[i]->setObjectName(QString());
    }

    update();
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

    stream >> slotCnt;
    for (int i = 0; i < slotCnt; ++i) {
        stream >> x >> y >> z;
        QGLBuilder builder;
        builder.newSection(QGL::Faceted);
        builder << QGLCube(6);
        builder.currentNode()->setY(3);

        MeshObject *box = new MeshObject(
                builder.finalizedSceneNode());
        box->setMaterial(boxMaterial);
        box->setPosition(QVector3D(x, y, z));
        box->setObjectId(i);
        box->setScale(0.5,1.0,1.0);
        boxes.push_back(box);
    }

    updateBoxes();
}

void View::hoverEnter(MeshObject *obj) {
    if (!obj) return;
    enteredObject = obj;
    if(!obj->objectName().isEmpty()) {
        qDebug() << obj->objectName();
        QVector3D pos = mvp * obj->position();
        drawText(pos.x(), pos.y(), obj->objectName());
        update();
    }
}

void View::hoverLeave() {
    /* TODO: clear text */
    //It's OK?
    enteredObject = NULL;
    drawText(0,0,"");
    update();
}

void View::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) {
        setOption(QGLView::ShowPicking, !(options() & QGLView::ShowPicking));
        update();
    } else if (event->key() == Qt::Key_R) {
        //TODO: reset camera
        camera()->setCenter(QVector3D(0, 50, 0));
        camera()->setEye(QVector3D(0, 50, 300));
        update();
    }
    QGLView::keyPressEvent(event);
}

void View::mouseDoubleClickEvent(QMouseEvent *event) {
    if (pickedObject && event->button() == Qt::LeftButton) {
        qDebug() << pickedObject->objectName();
        /* enter dir or open file */
        if (pickedObject->objectId() < dirEntryCnt) {
            hoverLeave();
            dir.cd(pickedObject->objectName());
            drawText(0, 0, "");
            updateBoxes();
        } else if (!QDesktopServices::openUrl("file:///"
                    + dir.absoluteFilePath(pickedObject->objectName())))
            qDebug() << "Open File Failed";

        pickedObject=NULL;
    }
}

void View::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        pickedObject = meshObjectAt(event->pos());
        if (pickedObject && pickedObject->pickType() == MeshObject::Pickable) {
            /* pick up object */
            pickedObject->setPickType(MeshObject::Picked);
            pickedPos = pickedObject->position();
            pickedDepth = (mvp * pickedPos).z();
            pickedModelPos = mvp.inverted()
                * extendTo3D(event->pos(), pickedDepth)
                - pickedPos;

            update();
            return;
        } else
            pickedObject = NULL;
    }

    QGLView::mousePressEvent(event);
}

void View::wheelEvent(QWheelEvent *) { }

void View::mouseReleaseEvent(QMouseEvent *event) {
    if (pickedObject && event->button() == Qt::LeftButton) {
        /* release picked object */
        MeshObject *anchor = meshObjectAt(event->pos());
        if (anchor) {
            QVector3D destPos = anchor->position();
            anchor->setPosition(pickedPos);
            pickedObject->setPosition(destPos);
        } else {
            pickedObject->setPosition(pickedPos);
        }

        pickedObject->setPickType(MeshObject::Pickable);
        pickedObject = NULL;

        update();
        return;
    }

    QGLView::mouseReleaseEvent(event);
}

void View::mouseMoveEvent(QMouseEvent *event) {
    if (pickedObject) {
        /* move picked object */
        pickedObject->setPosition(
                mvp.inverted() * extendTo3D(event->pos(), pickedDepth)
                - pickedModelPos);
        update();
        return;
    }
    
    MeshObject *object = meshObjectAt(event->pos());
    if (object != enteredObject) {
        hoverLeave();
        hoverEnter(object);
    }
   
    QGLView::mouseMoveEvent(event);
}

MeshObject* View::meshObjectAt(const QPoint &pos) {
    return qobject_cast<MeshObject*>( objectForPoint(pos) );
}

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size) {
    qreal w = size.width();
    qreal h = size.height();
    QMatrix4x4 cameraMvp =
        camera->projectionMatrix(w / h) * camera->modelViewMatrix();
    /* transform from (-1~1,-1~1) to (0~800,0~600) */
    QMatrix4x4 screenMvp = QMatrix4x4(
            w/2,   0,  0, w/2,
             0,  -h/2, 0, h/2,
             0,    0,  1,  0,
             0,    0,  0,  1
        ) * cameraMvp;
    return screenMvp;
}

static QVector3D extendTo3D(const QPoint &pos, qreal depth) {
    return QVector3D(pos.x(), pos.y(), depth);
}
