#include "view.h"
#include "imageobject.h"
#include "directory.h"
#include "common.h"
#include "room.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtCore/QVariantAnimation>
#include <QtGui/QDesktopServices>

inline QVector3D extendTo3D(const QPoint &pos, qreal depth)
{
    return QVector3D(pos.x(), pos.y(), depth);
}

void View::invokeObject(int id)
{
    if (pickedObject != -1) {
        if (id == TrashBin && dir->remove(pickedObject))
            loadDir();

    } else {
        switch (id) {
        case Picture:
            picture->setImage(dir->getNextImage());
            break;

        case Door:
            hoveringObject = -1;
            leavingDoor = id;
            dir->cdUp();
            curRoom->clearBack();
            startAnimation(Leaving1);
            break;

        case LeftArrow:
            dir->prevPage();
            loadDir();
            break;

        case RightArrow:
            dir->nextPage();
            loadDir();
            break;

        default:
            qDebug() << "Nothing happended";
        }
    }
}

void View::openEntry(int index)
{
    if (index < dir->countDir()) {
        hoverLeave();
        dir->cd(index);
        loadDir(true);
        enteringDir = index;
        startAnimation(Entering1);
    } else {
        QString path = dir->absoluteFilePath(dir->entry(index));
        if (!QDesktopServices::openUrl("file:///" + path))
            qDebug() << "Open File Failed";
    }
}

void View::mousePressEvent(QMouseEvent *event) {
    if (animStage != NoAnim || event->button() != Qt::LeftButton) return;
    int obj = objectIdForPoint(event->pos());

    if (obj >= 0 && obj < dir->count()) {
        pickedObject = obj;
        curRoom->pickEntry(obj);
        pickedPos = curRoom->getEntryMat(pickedObject) * QVector3D(0, 0, 0);
        //pickedPos = curRoom->getEntryPos(pickedObject);
        pickedDepth = (mvp * pickedPos).z();
        pickedModelPos = mvp.inverted() * extendTo3D(event->pos(), pickedDepth) - pickedPos;
        isNear = true;
        update();

    } else if (obj == -1) {
        roamStartCenter = camera()->center();
        roamStartPos = event->pos();
        isRoaming = true;
    }
}

void View::mouseReleaseEvent(QMouseEvent *event) {
    if (animStage != NoAnim || event->button() != Qt::LeftButton) return;

    if (isRoaming) {
        isRoaming = false;
        return;
    }

    if (isNear)
        openEntry(pickedObject);
    else {
        int id = objectIdForPoint(event->pos());
        if (id > dir->count())
            invokeObject(id);
    }

    curRoom->pickEntry(-1);
    pickedObject = -1;
    deltaPos = QVector3D(0, 0, 0);
    isNear = false;
    update();
}

void View::mouseMoveEvent(QMouseEvent *event) {
    if (animStage != NoAnim) return;

    if (isRoaming) {
        /* FIXME: moving mouse outside window may cause strange behaviour */
        /* The bug is caused by center() - eye() == (0, y, 0), which is parallel to up vector */
        QVector3D moveVector = (mvp.inverted() * QVector4D(event->pos() - roamStartPos)).toVector3D();
        QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(roamStartCenter, -moveVector), moveVector.length() * 40);
        camera()->setCenter(rotation.rotatedVector(roamStartCenter - QVector3D(0, eyeHeight, 0)) + QVector3D(0, eyeHeight, 0));
        return;
    }

    int obj = objectIdForPoint(event->pos());
    if (obj != hoveringObject) {
        hoverLeave();
        if (obj >= 0 && obj < StartImageId)
            hoverEnter(obj);
    }

    if (pickedObject != -1) {
        /* move picked object */
        deltaPos = mvp.inverted() * extendTo3D(event->pos(), pickedDepth) - pickedModelPos - pickedPos;
        update();
        return;
    }
}

void View::hoverEnter(int obj) {
    if (obj == -1) return;
    hoveringObject = obj;
    //if (obj->pickType() != MeshObject::Normal) return;
    paintOutline(obj);
    update();
}

void View::hoverLeave() {
    hoveringObject = -1;
    paintHud();
    update();
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
        setOption(GLView::ShowPicking, !(options() & GLView::ShowPicking));
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
        loadDir();
    } else if(event->key() == Qt::Key_Up) {
        hoverLeave();
        dir->prevPage();
        loadDir();
    } else if (event->key() == Qt::Key_Down) {
        hoverLeave();
        dir->nextPage();
        loadDir();
    }
    //GLView::keyPressEvent(event);
}

void View::wheelEvent(QWheelEvent *) { }
