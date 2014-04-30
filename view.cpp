#include "view.h"
#include "imageobject.h"
#include "meshobject.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QDesktopServices>

#include <QtCore/QDebug>

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);
static QVector3D extendTo3D(const QPoint &pos, qreal depth);

View::View(int width, int height)
    : pickedObject(NULL), enteredObject(NULL), hudObject(NULL), picture(NULL)
{
    resize(width, height);

    camera()->setCenter(QVector3D(0, 50, 0));
    camera()->setEye(QVector3D(0, 50, 300));

    mvp = calcMvp(camera(), size());

    /* background */
    QGLMaterial *shelfMaterial = new QGLMaterial();
    shelfMaterial->setAmbientColor(QColor(192, 150, 128));
    shelfMaterial->setSpecularColor(QColor(60, 60, 60));
    shelfMaterial->setShininess(128);

    MeshObject *shelf = new MeshObject(
            QGLAbstractScene::loadScene(":/model/shelf.obj"), MeshObject::Static);
    shelf->setPosition(QVector3D(0, 0, 0));
    shelf->setMaterial(shelfMaterial);

    MeshObject *frame = new MeshObject(
            QGLAbstractScene::loadScene(":/model/frame.obj"), MeshObject::Static);
    frame->setPosition(QVector3D(-50, 50, 0));
    frame->setMaterial(shelfMaterial);

    background << shelf << frame;

    /* boxes */
    initializeBox();

    /* HUD */
    hudObject = new ImageObject(2, 2, ImageObject::Hud);

    /* picture */
    picture = new ImageObject(30, 20);
    picture->setPosition(frame->position());
    picture->regist(this, boxes.size());

    updateDir();
}

View::~View() {
    for (auto obj : background) obj->deleteLater();
    for (auto obj : boxes) obj->deleteLater();
    delete picture;
    delete hudObject;
}

QImage View::paintHud(float x, float y, QString text) {
    QImage ret(width(), height(), QImage::Format_ARGB32_Premultiplied);
    ret.fill(Qt::transparent);
    QPainter painter(&ret);
    QFont font = painter.font();
    font.setPointSize(16);
    font.setBold(1);
    painter.setFont(font);
    painter.setPen(QColor(Qt::red));
    painter.drawText(x, y, text);
    painter.drawText(0, 20, dir.absolutePath());
    return ret;
}

void View::drawText(float x, float y, QString text) {
    hudObject->setImage(paintHud(x, y, text));
}

void View::resizeEvent(QResizeEvent *e) {
    drawText(0, 0, "");
    update();
}

void View::initializeGL(QGLPainter *painter) {
    for (auto obj : boxes) obj->initialize(this, painter);
}

void View::paintGL(QGLPainter *painter) {
    mvp = calcMvp(camera(), size());

    for (auto obj : background) obj->draw(painter);
    for (auto obj : boxes) obj->draw(painter);

    Q_ASSERT(picture != NULL);
    Q_ASSERT(hudObject != NULL);

    picture->draw(painter);
    hudObject->draw(painter);
}

void View::updateDir() {
    // Show picture
    static const QStringList filter = {
        "*.bmp", "*.jpg", "*.jpeg", "*.gif", "*.png" };
    pictureList = dir.entryList(filter, QDir::Files);
    currentPicture = 0;

    if (pictureList.isEmpty())
        picture->setImage(":/model/photo.png");
    else
        picture->setImage(dir.absoluteFilePath(pictureList[currentPicture]));

    /* update entry info */
    QStringList entryList = dir.entryList(QDir::AllEntries | QDir::NoDot,
            QDir::DirsFirst | QDir::IgnoreCase);
    entryCnt = entryList.size();
    dirEntryCnt = dir.entryList(QDir::Dirs | QDir::NoDot).size();

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

        MeshObject *box = new MeshObject(builder.finalizedSceneNode());
        box->setMaterial(boxMaterial);
        box->setPosition(QVector3D(x, y, z));
        box->setObjectId(i);
        box->setScale(0.5, 1.0, 1.0);
        boxes.push_back(box);
    }
}

void View::hoverEnter(MeshObject *obj) {
    if (!obj) return;
    enteredObject = obj;
    if (!obj->objectName().isEmpty()) {
        qDebug() << obj->objectName();
        QVector3D pos = mvp * obj->position();
        drawText(pos.x(), pos.y(), obj->objectName());
        update();
    }
}

void View::hoverLeave() {
    enteredObject = NULL;
    drawText(0, 0, "");
    update();
}

void View::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) {
        setOption(QGLView::ShowPicking, !(options() & QGLView::ShowPicking));
        update();
    } else if (event->key() == Qt::Key_R) {
        camera()->setCenter(QVector3D(0, 50, 0));
        camera()->setEye(QVector3D(0, 50, 300));
        camera()->setUpVector(QVector3D(0, 1, 0));
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
            updateDir();
        } else if (!QDesktopServices::openUrl(
                    "file:///" +
                    dir.absoluteFilePath(pickedObject->objectName())))
            qDebug() << "Open File Failed";

        pickedObject = NULL;
    }
}

void View::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QObject *obj = objectForPoint(event->pos());

        if (obj == picture) {
            nextPicture();
            return;
        }

        pickedObject = qobject_cast<MeshObject*>(obj);
        if (pickedObject && pickedObject->pickType() == MeshObject::Pickable) {
            /* pick up object */
            pickedObject->setPickType(MeshObject::Picked);
            pickedPos = pickedObject->position();
            pickedDepth = (mvp * pickedPos).z();
            pickedModelPos =
                mvp.inverted() * extendTo3D(event->pos(), pickedDepth) - pickedPos;

            update();
            return;
        } else
            pickedObject = NULL;
    }

    QGLView::mousePressEvent(event);
}

void View::wheelEvent(QWheelEvent *) {}

void View::mouseReleaseEvent(QMouseEvent *event) {
    if (pickedObject && event->button() == Qt::LeftButton) {
        /* release picked object */
        QObject *obj = objectForPoint(event->pos());
        if (obj == picture) return;

        MeshObject *anchor = qobject_cast<MeshObject*>(obj);
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
        pickedObject->setPosition(mvp.inverted() *
                extendTo3D(event->pos(), pickedDepth) -
                pickedModelPos);
        update();
        return;
    }

    QObject *object = objectForPoint(event->pos());
    if (object != enteredObject) {
        hoverLeave();
        if (object != picture)
            hoverEnter(qobject_cast<MeshObject*>(object));
    }

    QGLView::mouseMoveEvent(event);
}

void View::nextPicture() {
    /* TODO: improve performance */
    if (pictureList.empty()) return;
    if (++currentPicture >= pictureList.size())
        currentPicture -= pictureList.size();
    picture->setImage(dir.absoluteFilePath(pictureList[currentPicture]));
}

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size) {
    qreal w = size.width();
    qreal h = size.height();
    QMatrix4x4 cameraMvp =
        camera->projectionMatrix(w / h) * camera->modelViewMatrix();
    /* transform from (-1~1,-1~1) to (0~800,0~600) */
    QMatrix4x4 screenMvp = QMatrix4x4(w / 2, 0, 0, w / 2,
            0, -h / 2, 0, h / 2,
            0, 0, 1, 0,
            0, 0, 0, 1) *
        cameraMvp;
    return screenMvp;
}

static QVector3D extendTo3D(const QPoint &pos, qreal depth) {
    return QVector3D(pos.x(), pos.y(), depth);
}
