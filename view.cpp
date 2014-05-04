#include "view.h"
#include "imageobject.h"
#include "meshobject.h"
#include "directory.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QDesktopServices>
#include <QtGui/QPainterPath>
#include <QtCore/QVariantAnimation>

#include <QtCore/QDebug>

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);
static QVector3D extendTo3D(const QPoint &pos, qreal depth);

View::View(int width, int height) :
    pickedObject(NULL), enteredObject(NULL), hudObject(NULL), picture(NULL),
    enteringDir(NULL), isLeavingDir(false), isShowingFileName(false)
{
    resize(width, height);

    dir = new Directory;

    camera()->setCenter(QVector3D(0, 50, 0));
    camera()->setEye(QVector3D(0, 50, 300));

    mvp = calcMvp(camera(), size());

    loadModels();
    setupObjects();

    animation = new QVariantAnimation();
    animation->setStartValue(QVariant(static_cast<qreal>(0.0)));
    animation->setEndValue(QVariant(static_cast<qreal>(1.0)));
    animation->setDuration(500);

    connect(animation, &QVariantAnimation::valueChanged, [=](const QVariant &var) {
            animProg = var.toReal(); update(); });
    connect(animation, &QVariantAnimation::finished, this, &View::finishAnimation);

    loadDir(boxes, picture);
}

void View::paintHud(qreal x = 0, qreal y = 0, QString text = QString()) {
    QImage image(width(), height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = painter.font();
    font.setPointSize(16);
    font.setBold(2);

    QPainterPath path;
    if (!text.isEmpty()) path.addText(x, y, font, text);
    path.addText(0, 20, font, dir->absolutePath());
    painter.setBrush(QColor(Qt::white));
    painter.setPen(QColor(Qt::black));
    painter.drawPath(path);

    if (text.isEmpty() && isShowingFileName)
        for (int i = 0; i < dir->count(); ++i) {
            QVector3D pos = mvp * boxes[i]->position();
            QRect rect(pos.x() - 30, pos.y(), 60, 30);
            QString text = boxes[i]->objectName();
            painter.drawText(rect, Qt::AlignHCenter | Qt::TextWrapAnywhere,
                    boxes[i]->objectName());
        }
    
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

        for (auto obj : staticMeshes) obj->draw(painter);
        for (auto obj : backBoxes) obj->draw(painter);
        backPicture->draw(painter);

        painter->modelViewMatrix().pop();
    }

    for (auto obj : staticMeshes) obj->draw(painter);
    for (auto obj : boxes) 
        if (obj != enteringDir && obj != pickedObject)
            obj->draw(painter);
    picture->draw(painter);

    glClear(GL_DEPTH_BUFFER_BIT);
    if (pickedObject)
        pickedObject->draw(painter);

    if (!(enteringDir || isLeavingDir)) hudObject->draw(painter);
}

void View::loadDir(const QVector<MeshObject*> &boxes, ImageObject *picture) {
    picture->setImage(dir->getImage());

    /* update entry info */
    for (int i = 0; i < dir->count(); ++i) {
        boxes[i]->setPickType(MeshObject::Normal);
        boxes[i]->setObjectName(dir->entry(i));
        boxes[i]->setModel(i < dir->countDir() ? dirModel : fileModel);
    }
    for (int i = dir->count(); i < slotCnt; ++i) {
        boxes[i]->setPickType(MeshObject::Anchor);
        boxes[i]->setObjectName(QString());
        boxes[i]->setModel(dirModel);
    }

    paintHud();
    update();
}

void View::finishAnimation() {
    camera()->setCenter(startCenter);
    camera()->setEye(startEye);
    if (enteringDir) {
        for (int i = 0; i < boxes.size(); ++i) {
            boxes[i]->setPickType(backBoxes[i]->pickType());
            boxes[i]->setObjectName(backBoxes[i]->objectName());
            boxes[i]->setModel(backBoxes[i]->model());
        }
        picture->setImage(backPicture->getImage());
    }
    enteringDir = NULL;
    isLeavingDir = false;
    paintHud();
    update();
}

void View::hoverEnter(MeshObject *obj) {
    enteredObject = obj;
    QVector3D pos = mvp * obj->position();
    paintHud(pos.x(), pos.y(), obj->objectName());
    update();
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
        paintHud();
        update();
    } else if (event->key() == Qt::Key_Space) {
        isShowingFileName = !isShowingFileName;
        paintHud();
        update();
    }
    QGLView::keyPressEvent(event);
}

void View::mouseDoubleClickEvent(QMouseEvent *event) {
    if (enteringDir || isLeavingDir) return;
    if (pickedObject && event->button() == Qt::LeftButton) {
        /* enter dir or open file */
        if (pickedObject->objectId() < dir->countDir()) {
            hoverLeave();

            dir->cd(pickedObject->objectName());
            loadDir(backBoxes, backPicture);

            enteringDir = pickedObject;
            startCenter = camera()->center();
            startEye = camera()->eye();
            deltaCenter = camera()->center() * (0.05 - 1) + pickedObject->position();
            deltaEye = camera()->eye() * (0.05 - 1) + pickedObject->position();
            animation->start();

        } else if (!QDesktopServices::openUrl("file:///" +
                    dir->absoluteFilePath(pickedObject->objectName())))
            qDebug() << "Open File Failed";

        pickedObject = NULL;
    }
}

void View::mousePressEvent(QMouseEvent *event) {
    if (enteringDir || isLeavingDir) return;
    if (event->button() == Qt::LeftButton) {
        PickObject *obj = qobject_cast<PickObject*>(objectForPoint(event->pos()));
        if (!obj || obj->objectId() == -1) {
            paintHud();
            QGLView::mousePressEvent(event);
            return;
        }

        if (obj->objectId() == Picture) {
            picture->setImage(dir->getNextImage());
            return;
        }

        if (obj->objectId() == TrashBin) return;

        if (obj->objectId() == Door) {
            isLeavingDir = true;
            loadDir(backBoxes, backPicture);
            dir->cdUp();
            loadDir(boxes, picture);

            /* TODO: the leaving animation is temporary */
            startCenter = camera()->center();
            startEye = camera()->eye();
            deltaCenter = camera()->center() * (0.05 - 1) + boxes[0]->position();
            deltaEye = camera()->eye() * (0.05 - 1) + boxes[0]->position();
            animation->start();

            return;
        }

        if (obj->objectId() == LeftArrow) {
            dir->prevPage();
            loadDir(boxes, picture);
            return;
        }

        if (obj->objectId() == RightArrow) {
            dir->nextPage();
            loadDir(boxes, picture);
            return;
        }

        pickedObject = qobject_cast<MeshObject*>(obj);
        if (pickedObject->pickType() == MeshObject::Normal) {
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

    paintHud();
    QGLView::mousePressEvent(event);
}

void View::wheelEvent(QWheelEvent *e) { }

void View::mouseReleaseEvent(QMouseEvent *event) {
    if (enteringDir || isLeavingDir) return;
    if (pickedObject && event->button() == Qt::LeftButton) {
        QVector3D destPos = pickedPos;
        PickObject *obj = qobject_cast<PickObject*>(objectForPoint(event->pos()));
        if (obj && obj->objectId() != -1) {
            /* remove picked file or directory */
            if (obj->objectId() == TrashBin) {
                if (dir->remove(pickedObject->objectId())) {
                    qDebug()<<"success";
                    loadDir(boxes, picture);
                }

            /* move picked object */
            } else if (obj->objectId() < MaxBoxId) {
                MeshObject *anchor = qobject_cast<MeshObject*>(obj);
                destPos = anchor->position();
                anchor->setPosition(pickedPos);
            }
        }

        /* release object */
        pickedObject->setPickType(MeshObject::Normal);
        pickedObject->setPosition(destPos);
        pickedObject = NULL;
        update();
        return;
    }

    paintHud();
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

    PickObject *obj = qobject_cast<PickObject*>(objectForPoint(event->pos()));
    if (obj != enteredObject) {
        hoverLeave();
        if (obj && obj->objectId() != -1 && obj->objectId() < dir->count())
            hoverEnter(qobject_cast<MeshObject*>(obj));
    }

    QGLView::mouseMoveEvent(event);
}

void View::loadModels() {
    mat1 = new QGLMaterial(this);
    mat1->setAmbientColor(QColor(192, 150, 128));
    mat1->setSpecularColor(QColor(60, 60, 60));
    mat1->setShininess(128);

    mat2 = new QGLMaterial(this);
    mat2->setAmbientColor(QColor(255, 255, 255));
    mat2->setDiffuseColor(QColor(150, 150, 150));
    mat2->setSpecularColor(QColor(255, 255, 255));
    mat2->setShininess(128);

    QGLAbstractScene *model;
    MeshObject *mesh;

    /* shelf */
    model = QGLAbstractScene::loadScene(":/model/shelf.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    model->mainNode()->setPosition(QVector3D(0, 0, 0));
    staticMeshes << new MeshObject(model, this);

    /* photo frame */
    model = QGLAbstractScene::loadScene(":/model/frame.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    model->mainNode()->setPosition(QVector3D(-50, 50, 0));
    staticMeshes << new MeshObject(model, this);

    /* trash bin */
    model = QGLAbstractScene::loadScene(":/model/trash.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    QMatrix4x4 trashBinTrans;
    trashBinTrans.translate(QVector3D(-40, 0, 10));
    trashBinTrans.scale(0.2);
    model->mainNode()->setLocalTransform(trashBinTrans);
    staticMeshes << new MeshObject(model, this, TrashBin);

    /* door */
    model = QGLAbstractScene::loadScene(":/model/door.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    QMatrix4x4 doorTrans;
    doorTrans.translate(QVector3D(50, 36, 0));
    doorTrans.scale(10);
    model->mainNode()->setLocalTransform(doorTrans);
    staticMeshes << new MeshObject(model, this, Door);

    /* arrows */
    model = QGLAbstractScene::loadScene(":/model/leftarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat2);
    mesh = new MeshObject(model, this, LeftArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(-50, 90, 0));
    staticMeshes << mesh;

    model = QGLAbstractScene::loadScene(":/model/rightarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat2);
    mesh = new MeshObject(model, this, RightArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(50, 90, 0));
    staticMeshes << mesh;

    /* directory and file boxes */
    QGLBuilder dirBuilder;
    dirBuilder.newSection(QGL::Faceted);
    dirBuilder << QGLCube(6);
    dirBuilder.currentNode()->setY(3);
    dirModel = dirBuilder.finalizedSceneNode();
    dirModel->setParent(this);

    QMatrix4x4 trans;
    trans.scale(QVector3D(0.5, 1, 1));
    QGLBuilder fileBuilder;
    fileBuilder.newSection(QGL::Faceted);
    fileBuilder << QGLCube(6);
    fileBuilder.currentNode()->setY(3);
    fileBuilder.currentNode()->setLocalTransform(trans);
    fileModel = fileBuilder.finalizedSceneNode();
    fileModel->setParent(this);
}

void View::setupObjects() {
    /* directories and files */
    QFile file(":/model/shelf.slots");
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);
    qreal x, y, z;
    stream >> slotCnt;
    dir->setPageSize(slotCnt);
    for (int i = 0; i < slotCnt; ++i) {
        stream >> x >> y >> z;
        MeshObject *box = new MeshObject(dirModel, this, i);
        box->setMaterial(mat2);
        box->setPosition(QVector3D(x, y, z));
        boxes.push_back(box);

        MeshObject *backBox = new MeshObject(dirModel, this, -2);
        backBox->setMaterial(mat2);
        backBox->setPosition(QVector3D(x, y, z));
        backBoxes.push_back(backBox);
    }
    file.close();

    /* picture */
    picture = new ImageObject(30, 20, this, ImageObject::Normal);
    picture->setPosition(QVector3D(-50, 50, 1));

    backPicture = new ImageObject(30, 20, this, ImageObject::Background);
    backPicture->setPosition(QVector3D(-50, 50, 1));

    /* HUD */
    hudObject = new ImageObject(2, 2, this, ImageObject::Hud);
}

View::~View() {
    /* other members should be deleted by QObject system */
    delete animation;
    delete dir;
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
