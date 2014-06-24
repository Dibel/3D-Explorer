#include "view.h"
#include "common.h"
#include "imageobject.h"
#include "room.h"
#include <QtCore/QVariantAnimation>

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

    QVector3D pos;
    if (stage == Entering1 || stage == Entering2)
        pos = curRoom->getEntryMat(enteringDir) * QVector3D(0, 0, 0);
    //qDebug() << pos << getEntryPos(enteringDir);

    switch (stage) {
        case Entering1:
            endCenter = pos;
            endEye = pos + QVector3D(0, roomHeight * boxScale * 2, 0);
            //endCenter = curRoom->getEntryPos(enteringDir);
            //endEye = curRoom->getEntryPos(enteringDir) + QVector3D(0, roomHeight * boxScale * 2, 0);
            deltaUp = QVector3D(0, -1, -1);
            break;

        case Entering2:
            endCenter = defaultCenter * boxScale + pos;
            endEye = defaultEye * boxScale + pos;
            //endCenter = defaultCenter * boxScale + curRoom->getEntryPos(enteringDir);
            //endEye = defaultEye * boxScale + curRoom->getEntryPos(enteringDir);
            deltaUp = QVector3D(0, 1, 1);
            break;

        case Leaving1:
            endCenter = curRoom->getDoorPos() + defaultEye + rotateCcw(0, 0, -roomLength / 2, curRoom->getDoorAngle());
            endEye = curRoom->getDoorPos() + defaultEye;
            deltaUp = QVector3D(0, 0, 0);
            break;

        case Leaving2:
            endEye = (defaultEye - rotateCcw(curRoom->getOutPos(), curRoom->getDoorAngle() - curRoom->getOutAngle())) / boxScale;
            endCenter = endEye + rotateCcw(0, 0, -roomLength / 2, curRoom->getDoorAngle()) / boxScale;
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

void View::finishAnimation()
{
    static QVector3D endCenter;
    static QVector3D endEye;

    switch (animStage) {
        case Entering1:
            startAnimation(Entering2);
            break;

        case Leaving1:
            loadDir(true);
            startAnimation(Leaving2);
            break;

        case Entering2:
            camera()->setCenter(defaultCenter);
            camera()->setEye(defaultEye);

            curRoom->switchBackAndFront();
            picture->setImage(backPicture->getImage());
        
            animStage = NoAnim;
            enteringDir = -1;
            break;

        case Leaving2:
            camera()->setEye(defaultEye);
            camera()->setCenter(rotateCcw(defaultCenter, curRoom->getOutAngle()));
            leavingDoor = -1;

            curRoom->switchBackAndFront();
            picture->setImage(backPicture->getImage());

            animStage = Leaving3;
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
            qDebug() << "finishAnimation: Unkown stage!";
    }
}
