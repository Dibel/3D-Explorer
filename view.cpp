#include "view.h"
#include "imageobject.h"
#include "meshobject.h"
#include "directory.h"
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QOpenGLShaderProgram>
#include <QtCore/QVariantAnimation>
#include <QtCore/QDebug>

const qreal View::boxScale = 0.05;

View::View(int width, int height) :
    enteringDir(NULL), isLeavingDir(false), isRotating(false),
    pickedObject(NULL), enteredObject(NULL), fbo(NULL), surface(NULL),
    isShowingFileName(false)
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

    loadModels();
    setupObjects();

    light = new QGLLightParameters(this);
    light->setPosition(QVector3D(0, roomHeight * 0.8, 0));
    light->setAmbientColor(QColor(90, 90, 90));
    light2 = new QGLLightParameters(this);
    light2->setPosition(QVector3D(0, roomHeight * 0.8, 0));
    light2->setAmbientColor(QColor(90, 90, 90));
//    qDebug()<<light2->diffuseColor()<<light2->ambientColor();

    animation = new QVariantAnimation();
    animation->setStartValue(QVariant(static_cast<qreal>(0.0)));
    animation->setEndValue(QVariant(static_cast<qreal>(1.0)));
    animation->setDuration(1500);

    connect(animation, &QVariantAnimation::valueChanged, [=](const QVariant &var) {
            animProg = var.toReal(); update(); });
    connect(animation, &QVariantAnimation::finished, this, &View::finishAnimation);

    outline = new ImageObject(2, 2, this, ImageObject::Outline);

    loadDir(boxes, picture);
}

View::~View() {
    /* other members should be deleted by QObject system */
    delete animation;
    delete dir;
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

void View::hoverEnter(MeshObject *obj) {
    enteredObject = obj;
    if (enteredObject->pickType() != MeshObject::Normal) return;
    paintOutline(obj);
    update();
}

void View::hoverLeave() {
    enteredObject = NULL;
    paintHud();
    update();
}

void View::loadDir(const QVector<MeshObject*> &boxes, ImageObject *picture) {
    picture->setImage(dir->getImage());

    qDebug() << dir->count();

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
    static QVector3D endCenter;
    static QVector3D endEye;
    if (animationStage == 1) {
        endCenter = QVector3D(0, eyeHeight, -roomSize) * boxScale + enteringDir->position();
        endEye = QVector3D(0, eyeHeight, 0) * boxScale + enteringDir->position();

        startCenter = camera()->center();
        startEye = camera()->eye();
        startUp = QVector3D(0, 0, -1);


        deltaCenter = endCenter - startCenter;
        deltaEye = endEye - startEye;
        deltaUp = QVector3D(0, 1, 1);

        animationStage = 2;
        animation->start();
        return;
    }

    if (!isLeavingDir) {
        camera()->setCenter(QVector3D(0, eyeHeight, -roomSize));
        camera()->setEye(QVector3D(0, eyeHeight, 0));
        camera()->setUpVector(QVector3D(0, 1, 0));
    }
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

void View::initializeGL(QGLPainter *painter) {
    painter->addLight(light);
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

void View::resizeEvent(QResizeEvent *) {
    paintHud();
    update();
}

void View::wheelEvent(QWheelEvent *) { }
