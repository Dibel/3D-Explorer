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

static const QVector3D defaultCenter(0, View::eyeHeight, -View::roomSize);
static const QVector3D defaultEye(0, View::eyeHeight, 0);

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

    camera()->setCenter(defaultCenter);
    camera()->setEye(defaultEye);
    camera()->setNearPlane(roomSize * 0.015);
    camera()->setFarPlane(roomSize * 50);

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
            endCenter = QVector3D(50, eyeHeight, -roomSize * 2 - 10);
            endEye = QVector3D(50, eyeHeight, -roomSize - 10);
            deltaUp = QVector3D(0, 0, 0);
            break;

        case Leaving2:
            endCenter = (defaultCenter + QVector3D(0, -50, -roomSize * 0.9)) / boxScale;
            endEye = (defaultEye + QVector3D(0, -50, -roomSize * 0.9)) / boxScale;
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
            break;

        case Leaving1:
            startAnimation(Leaving2);
            break;

        case Entering2:
        case Leaving2:
            camera()->setCenter(QVector3D(0, eyeHeight, -roomSize));
            camera()->setEye(QVector3D(0, eyeHeight, 0));

            for (int i = 0; i < boxes.size(); ++i) {
                boxes[i]->setPickType(backBoxes[i]->pickType());
                boxes[i]->setObjectName(backBoxes[i]->objectName());
                boxes[i]->setModel(backBoxes[i]->model());
            }
            picture->setImage(backPicture->getImage());

            animStage = NoAnim;
            enteringDir = NULL;
            isLeavingDir = false;
            break;

        default:
            qDebug() << "finishAnimation: Unkown stage!";
    }
}

void View::debugFunc() {
    qDebug() << "done";
    update();
}

void View::initializeGL(QGLPainter *painter) {
    lightId = painter->addLight(light);
    qDebug() << "initial light id:" << lightId;
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
