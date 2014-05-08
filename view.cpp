#include "view.h"
#include "imageobject.h"
#include "meshobject.h"
#include "directory.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLFramebufferObjectSurface>
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QDesktopServices>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QPainterPath>
#include <QtCore/QVariantAnimation>
#include <QtCore/QDebug>

const qreal View::boxScale = 0.05;

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);
static QVector3D extendTo3D(const QPoint &pos, qreal depth = 0);
static QVector3D screenToView(const QPoint &screenPos);

class PickSurface : public QGLAbstractSurface {
public:
    PickSurface(QGLView *view, QOpenGLFramebufferObject *fbo, const QSize &areaSize) :
        QGLAbstractSurface(504), m_view(view), m_fbo(fbo),
        m_viewportGL(QPoint(0, 0), areaSize) { }

    QPaintDevice *device() const;
    bool activate(QGLAbstractSurface *) { if (m_fbo) m_fbo->bind(); return true; }
    void deactivate(QGLAbstractSurface *) { if (m_fbo) m_fbo->release(); }
    QRect viewportGL() const { return m_viewportGL; }
private:
    QGLView *m_view;
    QOpenGLFramebufferObject *m_fbo;
    QRect m_viewportGL;
};

View::View(int width, int height) :
    pickedObject(NULL), enteredObject(NULL), hudObject(NULL), picture(NULL), outline(NULL),
    enteringDir(NULL), isLeavingDir(false), isShowingFileName(false), isRotating(false),
    fbo(NULL), surface(NULL)
{
    phongEffect = new QGLShaderProgramEffect();
    phongEffect->setVertexShaderFromFile(":/shader/phong.vsh");
    phongEffect->setFragmentShaderFromFile(":/shader/phong.fsh");

    boxEffect = new QGLShaderProgramEffect();
    boxEffect->setVertexShaderFromFile(":/shader/box.vsh");
    boxEffect->setFragmentShaderFromFile(":/shader/box.fsh");
    resize(width, height);

    dir = new Directory;

    camera()->setCenter(QVector3D(0, eyeHeight, -roomSize));
    camera()->setEye(QVector3D(0, eyeHeight, 0));
    camera()->setNearPlane(roomSize * 0.015);

    mvp = calcMvp(camera(), size());

    loadModels();
    setupObjects();

    light = new QGLLightParameters(this);
    light->setPosition(QVector3D(0, roomHeight * 0.8, 0));
    light->setAmbientColor(QColor(120, 120, 120));
    light2 = new QGLLightParameters(this);
//    light2->setPosition(QVector3D(0, roomHeight * 0.8, -roomSize * 0.8));
//    light2->setDirection(QVector3D(0, 0, -1));
//    light2->setAmbientColor(QColor(255, 255, 255));
//    qDebug()<<light2->diffuseColor()<<light2->ambientColor();

    animation = new QVariantAnimation();
    animation->setStartValue(QVariant(static_cast<qreal>(0.0)));
    animation->setEndValue(QVariant(static_cast<qreal>(1.0)));
    animation->setDuration(2500);

    connect(animation, &QVariantAnimation::valueChanged, [=](const QVariant &var) {
            animProg = var.toReal(); update(); });
    connect(animation, &QVariantAnimation::finished, this, &View::finishAnimation);

    outline = new ImageObject(2, 2, this, ImageObject::Outline);

    loadDir(boxes, picture);
}

void View::paintHud(qreal x = 0, qreal y = 0, QString text = QString()) {
    QImage image(width(), height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);


    QFont font = painter.font();
    font.setPointSize(18);
    font.setBold(1);

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

    painter->addLight(light);
    //painter->addLight(light2);
    painter->setUserEffect(phongEffect);
    phongEffect->program()->setUniformValue("ambientColor", 0.2f, 0.2f, 0.2f, 1.0f);
    phongEffect->program()->setUniformValue("diffuseColor", 1.0f, 1.0f, 1.0f, 1.0f);
    phongEffect->program()->setUniformValue("specularColor", 1.0f, 1.0f, 1.0f, 1.0f);
    for (auto obj : staticMeshes) obj->draw(painter);
    floor->draw(painter);
    ceil->draw(painter);
    for (auto obj : boxes) 
        if (obj != enteringDir && obj != pickedObject) // && obj != enteredObject)
            obj->draw(painter);
    picture->draw(painter);

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
        painter->modelViewMatrix().scale(boxScale * 0.99);

        for (auto obj : staticMeshes) obj->draw(painter);
        for (auto obj : backBoxes) obj->draw(painter);
        backPicture->draw(painter);

        painter->modelViewMatrix().translate(0, 0.1, 0);
        floor->draw(painter);
        painter->modelViewMatrix().pop();
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    if (pickedObject)
        pickedObject->draw(painter);

    if (enteredObject && enteredObject->pickType() == MeshObject::Normal) outline->draw(painter);
    glClear(GL_DEPTH_BUFFER_BIT);
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
    if (animationStage == 1) {
        QVector3D endCenter = QVector3D(0, eyeHeight, -roomSize) * boxScale + enteringDir->position();
        QVector3D endEye = QVector3D(0, eyeHeight, 0) * boxScale + enteringDir->position();

        startCenter = camera()->center();
        startEye = camera()->eye();

        deltaCenter = endCenter - startCenter;
        deltaEye = endEye - startEye;

        animationStage = 2;
        animation->start();
        return;
    }

    camera()->setCenter(QVector3D(0, eyeHeight, -roomSize));
    camera()->setEye(QVector3D(0, eyeHeight, 0));
    if (enteringDir) {
        for (int i = 0; i < boxes.size(); ++i) {
            boxes[i]->setPickType(backBoxes[i]->pickType());
            boxes[i]->setObjectName(backBoxes[i]->objectName());
            boxes[i]->setModel(backBoxes[i]->model());
        }
        picture->setImage(backPicture->getImage());
    }

    pickedObject = NULL;
    enteringDir = NULL;
    isLeavingDir = false;
    paintHud();
    update();
}

void View::debugFunc() {
    qDebug() << "done";
    update();
}

void View::hoverEnter(MeshObject *obj) {
    enteredObject = obj;
    if (enteredObject->pickType() != MeshObject::Normal) return;

    QVector3D pos = mvp * obj->position();
    paintHud(pos.x(), pos.y(), obj->objectName());

    if (!fbo)
        fbo = new QOpenGLFramebufferObject(size(), QOpenGLFramebufferObject::CombinedDepthStencil);
    if (!surface)
        surface = new PickSurface(this, fbo, size());
    QGLPainter painter(this);
    painter.pushSurface(surface);
    painter.setPicking(true);
    painter.clearPickObjects();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    painter.setEye(QGL::NoEye);
    painter.setCamera(camera());

    enteredObject->ignoreMuting(true);
    PickObject::muteObjectId(true);

    paintGL(&painter);

    enteredObject->ignoreMuting(false);
    PickObject::muteObjectId(false);

    painter.setPicking(false);
    painter.popSurface();

    /* FIXME: use texture id */
    outline->setImage(fbo->toImage());

    update();
}

void View::hoverLeave() {
    //if (enteredObject)
    //qDebug() << "left" << enteredObject;
    //if(enteredObject != NULL) enteredObject->setEffect(0);
    enteredObject = NULL;
    paintHud();
    update();
}

void View::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) {
        setOption(QGLView::ShowPicking, !(options() & QGLView::ShowPicking));
        update();
    } else if (event->key() == Qt::Key_R) {
        camera()->setCenter(QVector3D(0, eyeHeight, 0));
        camera()->setEye(QVector3D(0, eyeHeight, roomSize));
        camera()->setUpVector(QVector3D(0, 1, 0));
        paintHud();
        update();
    } else if (event->key() == Qt::Key_Space) {
        isShowingFileName = !isShowingFileName;
        paintHud();
        update();
    } else if (event->key() == Qt::Key_D)
        debugFunc();
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

            /* FIXME: animation is broken */
            enteringDir = pickedObject;
            startCenter = camera()->center();
            startEye = camera()->eye();

            QVector3D endCenter = pickedObject->position();
            QVector3D endEye = pickedObject->position() + QVector3D(0, roomHeight * boxScale * 2, 0);

            deltaCenter = endCenter - startCenter;
            deltaEye = endEye - startEye;

            animationStage = 1;
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
            isRotating = true;
            pressPos = event->pos();
            oldCameraCenter = camera()->center();
            return;
        }

        if (obj->objectId() == Picture) {
            picture->setImage(dir->getNextImage());
            return;
        }

        if (obj->objectId() == TrashBin) return;

        if (obj->objectId() == Door) {
            enteredObject = NULL;
            isLeavingDir = true;
            loadDir(backBoxes, backPicture);
            dir->cdUp();
            loadDir(boxes, picture);

            /* TODO: the leaving animation is temporary */
            startCenter = camera()->center();
            startEye = camera()->eye();
            deltaCenter = camera()->center() * (boxScale - 1) + boxes[0]->position();
            deltaEye = camera()->eye() * (boxScale - 1) + boxes[0]->position();
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
    isRotating = true;
    pressPos = event->pos();
    oldCameraCenter = camera()->center();
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
    isRotating = false;
}

void View::mouseMoveEvent(QMouseEvent *event) {
    /* FIXME: screen flicks at the end of leaving directory animation if mouse is moving */
    if (enteringDir || isLeavingDir) return;

    if (isRotating) {
        /* FIXME: moving mouse outside window may cause strange behaviour */
        /* The bug is caused by center() - eye() == (0, y, 0), which is parallel to up vector */
        QVector3D moveVector = (mvp.inverted() * QVector4D(event->pos() - pressPos)).toVector3D();
        QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(oldCameraCenter, -moveVector), moveVector.length() * 40);
        camera()->setCenter(rotation.rotatedVector(oldCameraCenter - QVector3D(0, eyeHeight, 0)) + QVector3D(0, eyeHeight, 0));
        return;
    }

    PickObject *obj = qobject_cast<PickObject*>(objectForPoint(event->pos()));
    if (obj != enteredObject) {
        hoverLeave();
        if (obj && obj->objectId() != -1 && obj->objectId() < StartImageId)
            hoverEnter(qobject_cast<MeshObject*>(obj));
    }

    if (pickedObject) {
        /* move picked object */
        pickedObject->setPosition(mvp.inverted() *
                extendTo3D(event->pos(), pickedDepth) -
                pickedModelPos);
        update();
        return;
    }
}

void View::loadModels() {
    mat1 = new QGLMaterial(this);
    mat1->setAmbientColor(QColor(192, 150, 128));
    mat1->setSpecularColor(QColor(60, 60, 60));
    mat1->setShininess(20);

    mat2 = new QGLMaterial(this);
    mat2->setAmbientColor(QColor(255, 255, 255));
    mat2->setDiffuseColor(QColor(150, 150, 150));
    mat2->setSpecularColor(QColor(255, 255, 255));
    mat2->setShininess(20);

    QGLAbstractScene *model;
    MeshObject *mesh;

    /* shelf */
    model = QGLAbstractScene::loadScene(":/model/shelf.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    model->mainNode()->setPosition(QVector3D(0, 0, -roomSize));
    staticMeshes << new MeshObject(model, this);

    /* photo frame */
    model = QGLAbstractScene::loadScene(":/model/frame.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    model->mainNode()->setPosition(QVector3D(-50, 50, -roomSize));
    staticMeshes << new MeshObject(model, this);

    /* trash bin */
    model = QGLAbstractScene::loadScene(":/model/trash.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    QMatrix4x4 trashBinTrans;
    trashBinTrans.translate(QVector3D(-40, 0, 10 - roomSize));
    trashBinTrans.scale(0.2);
    model->mainNode()->setLocalTransform(trashBinTrans);
    staticMeshes << new MeshObject(model, this, TrashBin);

    /* door */
    model = QGLAbstractScene::loadScene(":/model/door.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    QMatrix4x4 doorTrans;
    doorTrans.translate(QVector3D(50, 36, -roomSize));
    doorTrans.scale(10);
    model->mainNode()->setLocalTransform(doorTrans);
    staticMeshes << new MeshObject(model, this, Door);

    /* arrows */
    model = QGLAbstractScene::loadScene(":/model/leftarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat2);
    mesh = new MeshObject(model, this, LeftArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(-50, 90, -roomSize));
    staticMeshes << mesh;

    model = QGLAbstractScene::loadScene(":/model/rightarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat2);
    mesh = new MeshObject(model, this, RightArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(50, 90, -roomSize));
    staticMeshes << mesh;

    /* directory and file boxes */
    QMatrix4x4 dirTrans;
    dirTrans.scale(QVector3D(roomSize * boxScale * 2, roomHeight * boxScale, roomSize * boxScale * 2));
    dirTrans.translate(QVector3D(0, 0.5, 0));
    QGLBuilder dirBuilder;
    dirBuilder.newSection(QGL::Faceted);
    dirBuilder << QGLCube(1);
    dirBuilder.currentNode()->setLocalTransform(dirTrans);
    //dirBuilder.currentNode()->setY(3);
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

    /* room, a cube that inside-out */
    QGLBuilder roomBuilder;
    roomBuilder.newSection(QGL::Faceted);
    roomBuilder.addPane(QSizeF(roomSize * 2, roomHeight));
    QGLSceneNode *pane = roomBuilder.finalizedSceneNode();
    QGLMaterial *roomMaterial = new QGLMaterial();
    roomMaterial->setAmbientColor(QColor(95, 75, 58));
    roomMaterial->setDiffuseColor(QColor(143, 122, 90));
    roomMaterial->setSpecularColor(QColor(154, 135, 105));
    roomMaterial->setShininess(10);
    pane->setMaterial(roomMaterial);
    //pane->setBackMaterial(mat2);
    pane->setPosition(QVector3D(0, roomHeight * 0.5, 0));

    MeshObject *front = new MeshObject(pane, this, -1);
    front->setPosition(QVector3D(0, 0, -roomSize));

    MeshObject *right = new MeshObject(pane, this, -1);
    right->setPosition(QVector3D(-roomSize, 0, 0));
    right->setRotationVector(QVector3D(0, 1, 0));
    right->setRotationAngle(90);

    MeshObject *back = new MeshObject(pane, this, -1);
    back->setPosition(QVector3D(0, 0, roomSize));
    back->setRotationVector(QVector3D(0, 1, 0));
    back->setRotationAngle(180);

    MeshObject *left = new MeshObject(pane, this, -1);
    left->setPosition(QVector3D(roomSize, 0, 0));
    left->setRotationVector(QVector3D(0, 1, 0));
    left->setRotationAngle(270);

    staticMeshes << front << right << back << left;

    QMatrix4x4 floorTrans;
    floorTrans.rotate(90, -1, 0, 0);
    QGLBuilder floorBuilder;
    floorBuilder.newSection(QGL::Faceted);
    floorBuilder.addPane(roomSize * 2);
    floorBuilder.currentNode()->setLocalTransform(floorTrans);
    QGLSceneNode *floorNode = floorBuilder.finalizedSceneNode();

    QGLTexture2D *floorTexture = new QGLTexture2D();
    QImage newImage;
    newImage.load(":/maps/floor.jpg");
    floorTexture->setImage(newImage);
    QGLMaterial *floorMaterial = new QGLMaterial();
    floorMaterial->setTexture(floorTexture);
    floorMaterial->setDiffuseColor(QColor(255, 255, 255));
    floorMaterial->setSpecularColor(QColor(255, 255, 255));
    floorMaterial->setShininess(60);
    floorMaterial->setTextureCombineMode(QGLMaterial::Decal);
    floorNode->setMaterial(floorMaterial);
    floorNode->setEffect(QGL::LitDecalTexture2D);
    floor = new MeshObject(floorNode, this, -1);

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(roomSize * 2);
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();

    QGLMaterial *ceilMaterial = new QGLMaterial();
    ceilMaterial->setAmbientColor(QColor(255, 255, 255));
    ceilMaterial->setDiffuseColor(QColor(255, 255, 255));
    ceilMaterial->setSpecularColor(QColor(255, 255, 255));
    ceilMaterial->setShininess(40);
    ceilNode->setMaterial(ceilMaterial);
    //ceilNode->setBackMaterial(roomMaterial);
    ceil = new MeshObject(ceilNode, this, -1);

    //staticMeshes << floor << ceil;

    ////QGLMaterial *roomMat = new QGLMaterial;
    ////roomMat->setColor(Qt::white);
    //QGLBuilder roomBuilder;
    //roomBuilder << QGLCube(300);
    //roomBuilder.currentNode()->setY(300);
    //roomBuilder.currentNode()->setMaterial(mat1);
    //roomBuilder.currentNode()->setBackMaterial(mat2);
    //QGLSceneNode *scene = roomBuilder.finalizedSceneNode();
    ////scene->setMaterial(roomMat);
    //mesh = new MeshObject(scene, this, -1);
    ////mesh->disableLight();
    //staticMeshes << mesh;
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
        box->setPosition(QVector3D(x, y, z - roomSize));
        boxes.push_back(box);

        MeshObject *backBox = new MeshObject(dirModel, this, -2);
        backBox->setMaterial(mat2);
        backBox->setPosition(QVector3D(x, y, z - roomSize));
        backBoxes.push_back(backBox);
    }
    file.close();

    /* picture */
    picture = new ImageObject(30, 20, this, ImageObject::Normal);
    picture->setPosition(QVector3D(-50, 50, 1 - roomSize));

    backPicture = new ImageObject(30, 20, this, ImageObject::Background);
    backPicture->setPosition(QVector3D(-50, 50, 1 - roomSize));

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
    QMatrix4x4 screenMvp = QMatrix4x4(
            w / 2, 0, 0, w / 2,
            0, -h / 2, 0, h / 2,
            0, 0, 1, 0,
            0, 0, 0, 1) *
        cameraMvp;
    return screenMvp;
}

static QVector3D extendTo3D(const QPoint &pos, qreal depth) {
    return QVector3D(pos.x(), pos.y(), depth);
}

static QVector3D screenToView(const QPoint &screenPos) {
    return QVector3D();
}
