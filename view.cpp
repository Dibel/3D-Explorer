#include "view.h"
#include "imageobject.h"
#include "meshobject.h"
#include "directory.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QDesktopServices>
#include <QtCore/QVariantAnimation>
#include <QtWidgets/QMessageBox>

#include <QtCore/QDebug>

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);
static QVector3D extendTo3D(const QPoint &pos, qreal depth);

View::View(int width, int height) :
    pickedObject(NULL), enteredObject(NULL), hudObject(NULL), picture(NULL),
    enteringDir(NULL), isLeavingDir(false), pageCnt(1)
{
    resize(width, height);

    dir = new Directory;

    camera()->setCenter(QVector3D(0, 50, 0));
    camera()->setEye(QVector3D(0, 50, 300));

    mvp = calcMvp(camera(), size());

    loadModels();
    setupMeshes();

    animation = new QVariantAnimation();
    animation->setStartValue(QVariant(static_cast<qreal>(0.0)));
    animation->setEndValue(QVariant(static_cast<qreal>(1.0)));
    animation->setDuration(1000);

    connect(animation, &QVariantAnimation::valueChanged, [=](const QVariant &var) {
            animProg = var.toReal(); update(); });
    connect(animation, &QVariantAnimation::finished, this, &View::finishAnimation);

    updateDir(boxes, picture);
}

void View::paintHud(qreal x = 0, qreal y = 0, QString text = QString()) {
    QImage image(width(), height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    QFont font = painter.font();
    font.setPointSize(16);
    font.setBold(1);
    painter.setFont(font);
    painter.setPen(QColor(Qt::red));
    painter.drawText(x, y, text);
    painter.drawText(0, 20, dir->absolutePath());
    hudObject->setImage(image);
}

void View::resizeEvent(QResizeEvent *e) {
    paintHud();
    update();
}

void View::initializeGL(QGLPainter *painter) {
    for (auto obj : boxes) obj->initialize(this, painter);
    for (auto obj : backBoxes) obj->initialize(this, painter);
}

void View::paintGL(QGLPainter *painter) {
    Q_ASSERT(picture != NULL);
    Q_ASSERT(hudObject != NULL);

    mvp = calcMvp(camera(), size());
    
    if (enteringDir || isLeavingDir) {
        if (painter->isPicking()) return;

        qreal t = animProg;
        if (isLeavingDir) t = 1.0 - t;
        if (t > 0.5) {
            t = 2 - t * 2;
            t = (2 - t * t) * 0.5;
        } else t = t * t * 2;
        camera()->setCenter(startCenter + t * deltaCenter);
        camera()->setEye(startEye + t * deltaEye);

        painter->modelViewMatrix().push();
        if (enteringDir)
            painter->modelViewMatrix().translate(enteringDir->position());
        else
            painter->modelViewMatrix().translate(boxes[0]->position());
        painter->modelViewMatrix().scale(0.05, 0.05, 0.05);

        for (auto obj : background) obj->draw(painter);
        for (auto obj : backBoxes) obj->draw(painter);
        backPicture->draw(painter);
        trashBin->draw(painter);
        door->draw(painter);

        painter->modelViewMatrix().pop();
    }

    for (auto obj : background) obj->draw(painter);
    for (auto obj : boxes) 
        if (obj != enteringDir && obj != pickedObject)
            obj->draw(painter);
    picture->draw(painter);
    trashBin->draw(painter);
    door->draw(painter);

    glClear(GL_DEPTH_BUFFER_BIT);
    if (pickedObject)
        pickedObject->draw(painter);

    if (!enteringDir) hudObject->draw(painter);
}

void View::updateDir(const QVector<MeshObject*> &boxes, ImageObject *picture) {
    dir->updateLists();

    picture->setImage(dir->getImage());

    /* update entry info */
    entryCnt = dir->count();
    dirEntryCnt = dir->countDir();

    int offset = 0;
    if (pageCnt > 1) {
        boxes[0]->setObjectName("上一页……");
        boxes[0]->setModel(dirModel);
        //boxes[0]->setScale(1.2, 1.2, 1.2);
        offset = 1;
    }

    int startPos;
    if (pageCnt == 1) {
        startPos = 0;
    } else {
        startPos = (pageCnt - 2) * (slotCnt - 2) + slotCnt - 1;
    }

    for (int i = 0; startPos + i < entryCnt && i + offset < slotCnt; ++i) {
        boxes[i + offset]->setPickType(MeshObject::Pickable);
        boxes[i + offset]->setObjectName(dir->entryList()[startPos + i]);
        boxes[i + offset]->setModel(startPos + i < dirEntryCnt ? dirModel : fileModel);
    }
    for (int i = entryCnt - startPos; i + offset < slotCnt; ++i) {
        boxes[i + offset]->setPickType(MeshObject::Anchor);
        boxes[i + offset]->setObjectName(QString());
        boxes[i + offset]->setModel(dirModel);
    }
    if(entryCnt > pageCnt * slotCnt) {
        boxes[slotCnt - 1]->setObjectName("下一页……");
        boxes[slotCnt - 1]->setModel(dirModel);
        //boxes[slotCnt - 1]->setScale(1.2, 1.2, 1.2);
    }
    paintHud();
    update();
}

void View::finishAnimation() {
    camera()->setCenter(startCenter);
    camera()->setEye(startEye);
    if (enteringDir) {
        qDebug() << "enteringDir:" << enteringDir;
        qDebug() << "leavingDir:" << isLeavingDir;
        for (int i = 0; i < boxes.size(); ++i) {
            boxes[i]->setPickType(backBoxes[i]->pickType());
            boxes[i]->setObjectName(backBoxes[i]->objectName());
            boxes[i]->setModel(backBoxes[i]->model());
        }
        picture->setImage(backPicture->getImage());
    }
    enteringDir = NULL;
    isLeavingDir = false;
    update();
}

void View::hoverEnter(MeshObject *obj) {
    if (!obj) return;
    enteredObject = obj;
    if (!obj->objectName().isEmpty()) {
        QVector3D pos = mvp * obj->position();
        paintHud(pos.x(), pos.y(), obj->objectName());
        update();
    }
}

void View::hoverLeave() {
    enteredObject = NULL;
    paintHud();
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
        /* enter dir or open file */
        int startPos;
        if (pageCnt == 1) {
            startPos = 0;
        } else {
            startPos = (pageCnt - 2) * (slotCnt - 2) + slotCnt - 1;
        }

        if (pickedObject->objectId() < (dirEntryCnt - startPos)) {
            hoverLeave();

            dir->cd(pickedObject->objectName());
            updateDir(backBoxes, backPicture);

            paintHud();
            pageCnt = 1;
            enteringDir = pickedObject;
            startCenter = camera()->center();
            startEye = camera()->eye();
            deltaCenter = camera()->center() * (0.05 - 1) + pickedObject->position();
            deltaEye = camera()->eye() * (0.05 - 1) + pickedObject->position();
            animation->start();

        } else if (!QDesktopServices::openUrl(
                    "file:///" +
                    dir->absoluteFilePath(pickedObject->objectName())))
            qDebug() << "Open File Failed";

        pickedObject = NULL;
    }
}

void View::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QObject *obj = objectForPoint(event->pos());

        if (obj == picture) {
            picture->setImage(dir->getNextImage());
            return;
        }

        if (obj == trashBin) return;

        if (obj == door) {
            isLeavingDir = true;
            updateDir(backBoxes, backPicture);
            dir->cdUp();
            updateDir(boxes, picture);

            /* TODO: merge with entering animation */
            paintHud();
            pageCnt = 1;
            startCenter = camera()->center();
            startEye = camera()->eye();
            deltaCenter = camera()->center() * (0.05 - 1) + boxes[0]->position();
            deltaEye = camera()->eye() * (0.05 - 1) + boxes[0]->position();
            animation->start();

            return;
        }

        pickedObject = qobject_cast<MeshObject*>(obj);
        if (pickedObject && pickedObject->pickType() == MeshObject::Pickable) {
            if (pickedObject->objectName() == "下一页……") {
                pageCnt++;
                pickedObject = NULL;
                updateDir(boxes, picture);
                return;
            } else if (pickedObject->objectName() == "上一页……") {
                pageCnt--;
                pickedObject = NULL;
                updateDir(boxes, picture);
                return;
            }
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

void View::wheelEvent(QWheelEvent *e) {
    //QGLView::wheelEvent(e);
}

void View::mouseReleaseEvent(QMouseEvent *event) {
    if (pickedObject && event->button() == Qt::LeftButton) {
        /* release picked object */
        QObject *obj = objectForPoint(event->pos());
        if (obj == picture || obj == door) return;
        //Delete the file
        if (obj == trashBin) {
            if (QMessageBox::question(NULL, "确认", "确认要删除吗？", QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
                if (pickedObject->objectName() != ".." && (pickedObject->objectId() < dirEntryCnt
                        ? QDir(dir->absoluteFilePath(pickedObject->objectName())).removeRecursively()
                        : dir->remove(pickedObject->objectName())))
                {
                    qDebug()<<"success";
                    hoverLeave();
                    pickedObject->setPickType(MeshObject::Anchor);
                    pickedObject->setObjectName(QString());
                    pickedObject->setModel(dirModel);
                }
            } else
                pickedObject->setPickType(MeshObject::Pickable);

            pickedObject->setPosition(pickedPos);
            pickedObject = NULL;

            update();
            return;
        }

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
    /* FIXME: screen flicks at the end of leaving directory animation if mouse is moving */
    if (enteringDir || isLeavingDir) return;

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

void View::loadModels() {
    mat1 = new QGLMaterial();
    mat1->setAmbientColor(QColor(192, 150, 128));
    mat1->setSpecularColor(QColor(60, 60, 60));
    mat1->setShininess(128);

    mat2 = new QGLMaterial();
    mat2->setAmbientColor(QColor(255, 255, 255));
    mat2->setDiffuseColor(QColor(150, 150, 150));
    mat2->setSpecularColor(QColor(255, 255, 255));
    mat2->setShininess(128);

    QGLAbstractScene *shelfModel = QGLAbstractScene::loadScene(":/model/shelf.obj");
    shelfModel->mainNode()->setMaterial(mat1);
    shelfModel->mainNode()->setPosition(QVector3D(0, 0, 0));
    MeshObject *shelf = new MeshObject(shelfModel, MeshObject::Static);

    QGLAbstractScene *frameModel = QGLAbstractScene::loadScene(":/model/frame.obj");
    frameModel->mainNode()->setMaterial(mat1);
    frameModel->mainNode()->setPosition(QVector3D(-50, 50, 0));
    MeshObject *frame = new MeshObject(frameModel, MeshObject::Static);

    backgroundModels << shelfModel << frameModel;

    trashBinModel = QGLAbstractScene::loadScene(":/model/trash.obj");
    trashBinModel->mainNode()->setMaterial(mat1);
    QMatrix4x4 trashBinTrans;
    trashBinTrans.translate(QVector3D(-40, 0, 10));
    trashBinTrans.scale(0.2);
    trashBinModel->mainNode()->setLocalTransform(trashBinTrans);

    doorModel = QGLAbstractScene::loadScene(":/model/door.obj");
    doorModel->mainNode()->setMaterial(mat1);
    QMatrix4x4 doorTrans;
    doorTrans.translate(QVector3D(50, 36, 0));
    doorTrans.scale(10);
    doorModel->mainNode()->setLocalTransform(doorTrans);

    QGLBuilder dirBuilder;
    dirBuilder.newSection(QGL::Faceted);
    dirBuilder << QGLCube(6);
    dirBuilder.currentNode()->setY(3);
    dirModel = dirBuilder.finalizedSceneNode();

    QMatrix4x4 trans;
    trans.scale(QVector3D(0.5, 1, 1));
    QGLBuilder fileBuilder;
    fileBuilder.newSection(QGL::Faceted);
    fileBuilder << QGLCube(6);
    fileBuilder.currentNode()->setY(3);
    fileBuilder.currentNode()->setLocalTransform(trans);
    fileModel = fileBuilder.finalizedSceneNode();
}

void View::setupMeshes() {
    /* background items */
    for (int i = 0; i < backgroundModels.size(); ++i)
        background.push_back(new MeshObject(backgroundModels[i], MeshObject::Static));

    /* trash bin */
    trashBin = new MeshObject(trashBinModel, MeshObject::Pickable);
    trashBin->setObjectId(-2);
    trashBin->setObjectName("垃圾桶");
    registerObject(-2, trashBin);

    /* door */
    door = new MeshObject(doorModel, MeshObject::Pickable);
    door->setObjectId(-3);
    door->setObjectName("..");
    registerObject(-3, door);

    /* directories and files */
    QFile file(":/model/shelf.slots");
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);
    qreal x, y, z;
    stream >> slotCnt;
    for (int i = 0; i < slotCnt; ++i) {
        stream >> x >> y >> z;
        MeshObject *box = new MeshObject(dirModel);
        box->setMaterial(mat2);
        box->setPosition(QVector3D(x, y, z));
        box->setObjectId(i);
        boxes.push_back(box);

        MeshObject *backBox = new MeshObject(dirModel);
        backBox->setMaterial(mat2);
        backBox->setPosition(QVector3D(x, y, z));
        backBoxes.push_back(backBox);
    }
    file.close();

    /* picture */
    picture = new ImageObject(30, 20);
    picture->setPosition(QVector3D(-50, 50, 1));
    picture->regist(this, boxes.size());
    backPicture = new ImageObject(30, 20);
    backPicture->setPosition(QVector3D(-50, 50, 1));

    /* HUD */
    hudObject = new ImageObject(2, 2, ImageObject::Hud);
}

View::~View() {
    for (auto obj : background) obj->deleteLater();
    for (auto obj : boxes) obj->deleteLater();
    for (auto obj : backBoxes) obj->deleteLater();
    trashBin->deleteLater();

    for (auto model : backgroundModels) delete model;
    delete trashBinModel;
    delete dirModel;
    delete fileModel;

    delete picture;
    delete backPicture;
    delete hudObject;

    delete animation;
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
