#include "view.h"
#include "common.h"
#include "imageobject.h"
#include "meshobject.h"
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
            endCenter = leavingDoor->position() + defaultEye + rotateCcw(0, 0, -roomLength / 2, leavingDoor->rotationAngle());
            endEye = leavingDoor->position() + defaultEye;
            deltaUp = QVector3D(0, 0, 0);
            break;

        case Leaving2:
            endEye = (defaultEye - rotateCcw(curRoom->getOutPos(), leavingDoor->rotationAngle() - curRoom->getOutAngle())) / boxScale;
            endCenter = endEye + rotateCcw(0, 0, -roomLength / 2, leavingDoor->rotationAngle()) / boxScale;
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

            curRoom->pushToFront();
            picture->setImage(backPicture->getImage());
        
            animStage = NoAnim;
            enteringDir = NULL;
            break;

        case Leaving2:
            camera()->setEye(defaultEye);
            camera()->setCenter(rotateCcw(defaultCenter, curRoom->getOutAngle()));
            leavingDoor = NULL;

            curRoom->pushToFront();
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
