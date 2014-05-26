#include "view.h"
#include "imageobject.h"
#include "meshobject.h"
#include "directory.h"
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QOpenGLShaderProgram>
#include <QtCore/QVariantAnimation>
#include <QtCore/QDebug>

const int View::roomSize = 80;
const int View::roomHeight = 120;
const int View::eyeHeight = 50;
const qreal View::boxScale = 0.05;

const QVector3D View::defaultCenter(0, eyeHeight, -roomSize);
const QVector3D View::defaultEye(0, eyeHeight, 0);

View::View(int width, int height) :
    enteringDir(NULL), leavingDoor(NULL), isRotating(false),
    pickedObject(NULL), enteredObject(NULL), fbo(NULL), surface(NULL),
    isShowingFileName(false)//, palette(new QGLMaterialCollection())
{
    phongEffect = new QGLShaderProgramEffect();
    phongEffect->setVertexShaderFromFile(":/shader/phong.vsh");
    phongEffect->setFragmentShaderFromFile(":/shader/phong.fsh");

    boxEffect = new QGLShaderProgramEffect();
    boxEffect->setVertexShaderFromFile(":/shader/box.vsh");
    boxEffect->setFragmentShaderFromFile(":/shader/box.fsh");
    resize(width, height);

    dir = new Directory;

    camera()->setCenter(defaultCenter);
    camera()->setEye(defaultEye);
    camera()->setNearPlane(roomSize * 0.015);
    camera()->setFarPlane(roomSize * 50);

    slotCnt = 0;
    loadModels();
    setupObjects();

    light = new QGLLightParameters(this);
    light->setPosition(QVector3D(0, roomHeight * 0.8, 0));
    light->setAmbientColor(QColor(120, 120, 120));

    animStage = NoAnim;
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
    /* picture */
    picture = new ImageObject(30, 20, this, ImageObject::Normal);
    picture->setPosition(QVector3D(-50, 50, 1 - roomSize));

    backPicture = new ImageObject(30, 20, this, ImageObject::Background);
    backPicture->setPosition(QVector3D(-50, 50, 1 - roomSize));

    /* HUD */
    hudObject = new ImageObject(2, 2, this, ImageObject::Hud);

    dir->setPageSize(slotCnt);
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

void View::startAnimation(AnimStage stage) {
    QVector3D endCenter;
    QVector3D endEye;
    QVector3D tmp1, tmp2;

    startCenter = camera()->center();
    startEye = camera()->eye();
    startUp = camera()->upVector();

    switch (stage) {
        case Entering1:
            endCenter = enteringDir->position();
            endEye = enteringDir->position() + QVector3D(0, roomHeight * boxScale * 2, 0);
            deltaUp = QVector3D(0, -1, -1);
            break;

        case Entering2:
            endCenter = defaultCenter * boxScale + enteringDir->position();
            endEye = defaultEye * boxScale + enteringDir->position();
            deltaUp = QVector3D(0, 1, 1);
            break;

        case Leaving1:
            tmp1 = rotate(leavingDoor->info().toInt(), 0, 0, leavingDoor->rotationAngle());
            tmp2 = rotate(0, 0, -roomSize, leavingDoor->rotationAngle());
            endCenter = leavingDoor->position() + defaultEye - tmp1 + tmp2;
            endEye = leavingDoor->position() + defaultEye - tmp1;
            deltaUp = QVector3D(0, 0, 0);
            break;

        case Leaving2:
            endEye = (defaultEye - rotate(cdUpPosition, leavingDoor->rotationAngle() - cdUpDirection)) / boxScale;
            endCenter = endEye + rotate(0, 0, -roomSize, leavingDoor->rotationAngle()) / boxScale;
            deltaUp = QVector3D(0, 0, 0);
            break;

        default:
            qDebug() << "startAnimation: Unknown stage!";
    }

    deltaCenter = endCenter - startCenter;
    deltaEye = endEye - startEye;
    animStage = stage;
    animation->start();
}

void View::finishAnimation() {
    static QVector3D endCenter;
    static QVector3D endEye;

    switch (animStage) {
        case Entering1:
            startAnimation(Entering2);
            return;

        case Leaving1:
            loadDir(backBoxes, backPicture);
            startAnimation(Leaving2);
            return;

        case Entering2:
            camera()->setCenter(defaultCenter);
            camera()->setEye(defaultEye);
            break;

        case Leaving2:
            camera()->setEye(defaultEye);
            camera()->setCenter(rotate(defaultCenter, cdUpDirection));
            leavingDoor = NULL;

            for (int i = 0; i < boxes.size(); ++i) {
                boxes[i]->setPickType(backBoxes[i]->pickType());
                boxes[i]->setObjectName(backBoxes[i]->objectName());
                boxes[i]->setModel(backBoxes[i]->model());
            }
            picture->setImage(backPicture->getImage());

            //animStage = NoAnim;
            animStage = Leaving3;
            animation->start();
            return;

        case Leaving3:
            animStage = NoAnim;
            camera()->setCenter(defaultCenter);
            camera()->setEye(defaultEye);
            return;

        default:
            qDebug() << "finishAnimation: Unkown stage!";
    }

    for (int i = 0; i < boxes.size(); ++i) {
        boxes[i]->setPickType(backBoxes[i]->pickType());
        boxes[i]->setObjectName(backBoxes[i]->objectName());
        boxes[i]->setModel(backBoxes[i]->model());
    }
    picture->setImage(backPicture->getImage());


    animStage = NoAnim;
    enteringDir = NULL;
}

void View::debugFunc() {
    qDebug() << "done";
    update();
}

void View::initializeGL(QGLPainter *painter) {
    lightId = painter->addLight(light);
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

QVector3D View::rotate(QVector3D vec, qreal angle) {
    return QQuaternion::fromAxisAndAngle(0, 1, 0, angle).rotatedVector(vec);
}

QVector3D View::rotate(qreal x, qreal y, qreal z, qreal angle) {
    return rotate(QVector3D(x, y, z), angle);
}
