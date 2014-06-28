#include "view.h"
#include "common.h"
#include "room.h"
#include <QtCore/QVariantAnimation>

inline QVector3D rotateCcw(QVector3D vec, qreal angle)
{
    return QQuaternion::fromAxisAndAngle(0, 1, 0, angle).rotatedVector(vec);
}

inline QVector3D rotateCcw(qreal x, qreal y, qreal z, qreal angle)
{
    return rotateCcw(QVector3D(x, y, z), angle);
}

void View::setupAnimation()
{
    animStage = NoAnim;

    animation = new QVariantAnimation();
    animation->setStartValue(QVariant(static_cast<qreal>(0.0)));
    animation->setEndValue(QVariant(static_cast<qreal>(1.0)));
    animation->setDuration(1500);

    connect(animation, &QVariantAnimation::valueChanged,
            [=](const QVariant &var)
            {
                animProg = var.toReal();
                update();
            });

    connect(animation, &QVariantAnimation::finished, this,
            &View::finishAnimation);
}

void View::startAnimation(AnimStage stage)
{
    static QVector3D endCenter;
    static QVector3D endEye;

    startCenter = camera()->center();
    startEye = camera()->eye();
    startUp = camera()->upVector();

    switch (stage) {
    case Entering1:
        endCenter = curRoom->getEntryPos(enteringDir);
        endEye = endCenter + QVector3D(0, roomHeight * boxScale * 2, 0);
        deltaUp = QVector3D(0, -1, -1);
        break;

    case Entering2:
        endCenter = defaultCenter * boxScale + curRoom->getEntryPos(enteringDir);
        endEye = defaultEye * boxScale + curRoom->getEntryPos(enteringDir);
        deltaUp = QVector3D(0, 1, 1);
        break;

    case Leaving1:
        endEye = curRoom->getDoorPos() + defaultEye;
        endCenter = endEye + rotateCcw(0, 0, -roomLength / 2, curRoom->getDoorAngle());
        deltaUp = QVector3D(0, 0, 0);
        break;

    case Leaving2:
        endEye = (defaultEye - rotateCcw(curRoom->getOutPos(), curRoom->getDoorAngle() - curRoom->getOutAngle())) / boxScale;
        endCenter = endEye + rotateCcw(0, 0, -roomLength / 2, curRoom->getDoorAngle()) / boxScale;
        deltaUp = QVector3D(0, 0, 0);
        break;

    case TurningLeft:
    case TurningRight:
        animation->setDuration(500);
        break;

    default:
        break;
    }

    animStage = stage;
    deltaCenter = endCenter - startCenter;
    deltaEye = endEye - startEye;
    animation->start();
}

void View::finishAnimation()
{
    static QVector3D endCenter;
    static QVector3D endEye;

    switch (animStage) {
    case Entering1:
        startAnimation(Entering2);
        break;

    case Entering2:
        camera()->setCenter(defaultCenter);
        camera()->setEye(defaultEye);
        curRoom->switchBackAndFront();
        animStage = NoAnim;
        enteringDir = -1;
        updateHudContent();
        update();
        break;

    case Leaving1:
        curRoom->loadBack(dir);
        startAnimation(Leaving2);
        break;

    case Leaving2:
        camera()->setEye(defaultEye);
        camera()->setCenter(rotateCcw(defaultCenter, curRoom->getOutAngle()));
        animStage = Leaving3;
        leavingDoor = -1;
        curRoom->switchBackAndFront();
        updateHudContent();
        animation->start();
        break;

    case Leaving3:
        animStage = NoAnim;
        camera()->setCenter(defaultCenter);
        camera()->setEye(defaultEye);
        break;

    case TurningLeft:
    case TurningRight:
        animStage = NoAnim;
        animation->setDuration(1500);
        break;

    default:
        break;
    }
}
