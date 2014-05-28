#include "view.h"
#include "meshobject.h"
#include "imageobject.h"
#include "directory.h"
#include "common.h"
#include <QtCore/QVariantAnimation>
#include <QtGui/QDesktopServices>

static QVector3D extendTo3D(const QPoint &pos, qreal depth) {
    return QVector3D(pos.x(), pos.y(), depth);
}

void View::invokeObject(PickObject *obj) {
    Q_ASSERT(obj && obj->objectId() != -1);

    switch (obj->objectId()) {
        case TrashBin:
            if (pickedObject && dir->remove(pickedObject->objectId()))
                loadDir(curRoom->entry, picture);
            break;

        case Picture:
            if (!pickedObject)
                picture->setImage(dir->getNextImage());
            break;

        case Door:
            if (!pickedObject) {
                enteredObject = NULL;
                leavingDoor = qobject_cast<MeshObject*>(obj);
                dir->cdUp();
                for (MeshObject *obj : curRoom->backEntry)
                    obj->setPickType(MeshObject::Anchor);
                startAnimation(Leaving1);
            } else {
                /* TODO: move file to parent directory */
            }
            break;

        case LeftArrow:
            if (!pickedObject) {
                dir->prevPage();
                loadDir(curRoom->entry, picture);
            }
            break;

        case RightArrow:
            if (!pickedObject) {
                dir->nextPage();
                loadDir(curRoom->entry, picture);
            }
            break;

        default:
            qDebug() << "Nothing happended";
    }
}

void View::mousePressEvent(QMouseEvent *event) {
    if (animStage != NoAnim || event->button() != Qt::LeftButton) return;
    PickObject *obj = qobject_cast<PickObject*>(objectForPoint(event->pos()));
    pressPos = event->pos();

    if (obj && obj->objectId() >= 0 && obj->objectId() < dir->count()) {
        pickedObject = qobject_cast<MeshObject*>(obj);
        pickedObject->setPickType(MeshObject::Picked);
        pickedPos = pickedObject->position();
        pickedDepth = (mvp * pickedPos).z();
        pickedModelPos = mvp.inverted() * extendTo3D(event->pos(), pickedDepth) - pickedPos;
        isNear = true;
        update();

    } else if (!obj || obj->objectId() <= MaxBoxId) {
        oldCameraCenter = camera()->center();
        isRotating = true;
    }
}

void View::mouseReleaseEvent(QMouseEvent *event) {
    if (animStage != NoAnim || event->button() != Qt::LeftButton) return;

    if (isRotating) {
        isRotating = false;
        return;
    }

    if (pickedObject) {
        pickedObject->setPickType(MeshObject::Normal);
        pickedObject->setPosition(pickedPos);
    }

    PickObject *obj = qobject_cast<PickObject*>(objectForPoint(event->pos()));
    if (obj && obj->objectId() != -1) {
        if (obj->objectId() < dir->count() && event->pos() == pressPos)
            openEntry(qobject_cast<MeshObject*>(obj));
        else
            invokeObject(obj);
    }

    pickedObject = NULL;
    update();
}

void View::mouseMoveEvent(QMouseEvent *event) {
    if (animStage != NoAnim) return;

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

void View::keyPressEvent(QKeyEvent *event) {
    if (animStage != NoAnim) return;
    if (event->key() == Qt::Key_Left) {
        hoverLeave();
        startCenter = camera()->center();
        animStage = TurningLeft;
        animation->setDuration(500);
        animation->start();
    } else if (event->key() == Qt::Key_Right) {
        hoverLeave();
        startCenter = camera()->center();
        animStage = TurningRight;
        animation->setDuration(500);
        animation->start();
    } if (event->key() == Qt::Key_Tab) {
        setOption(QGLView::ShowPicking, !(options() & QGLView::ShowPicking));
        update();
    } else if (event->key() == Qt::Key_R) {
        hoverLeave();
        camera()->setCenter(defaultCenter);
        camera()->setEye(defaultEye);
        camera()->setNearPlane(roomLength / 2 * 0.015);
        camera()->setFarPlane(roomLength / 2 * 50);
        camera()->setUpVector(QVector3D(0, 1, 0));
        paintHud();
        update();
    } else if (event->key() == Qt::Key_Space) {
        isShowingFileName = !isShowingFileName;
        paintHud();
        update();
    } else if (event->key() == Qt::Key_D) {
        hoverLeave();
        camera()->setCenter(QVector3D(0, eyeHeight, roomLength / 2));
        camera()->setEye(defaultEye);
        camera()->setNearPlane(roomLength / 2 * 0.015);
        camera()->setFarPlane(roomLength / 2 * 50);
        camera()->setUpVector(QVector3D(0, 1, 0));
        paintHud();
        update();
        //debugFunc();
    } else if (event->key() == Qt::Key_U) {
        hoverLeave();
        dir->cdUp();
        loadDir(curRoom->entry, picture);
    }
    //QGLView::keyPressEvent(event);
}

void View::wheelEvent(QWheelEvent *) { }

void View::openEntry(MeshObject *obj) {
    Q_ASSERT(obj && obj->objectId() != -1);
    if (obj->objectId() < dir->countDir()) {
        hoverLeave();
        dir->cd(obj->objectName());
        loadDir(curRoom->backEntry, backPicture);

        enteringDir = obj;
        startAnimation(Entering1);
    } else {
        QString path = dir->absoluteFilePath(obj->objectName());
        if (!QDesktopServices::openUrl("file:///" + path))
            qDebug() << "Open File Failed";
    }
}
