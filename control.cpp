#include "view.h"
#include "common.h"
#include "directory.h"
#include "room.h"
#include <QtGui/QDesktopServices>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>

inline QVector3D extendTo3D(const QPoint &pos, qreal depth)
{
    return QVector3D(pos.x(), pos.y(), depth);
}

inline void openFile(const QString &path)
{
    if (!QDesktopServices::openUrl("file:///" + path))
        qDebug() << "Open File Failed";
}

inline QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);

void View::invokeObject(int id)
{
    if (pickedEntry != -1) {
        switch (id) {
        case TrashBin:
            if (dir->remove(pickedEntry)) {
                curRoom->loadFront(dir);
                update();
            }
            hoverLeave();
            break;

        case Image:
            curRoom->setImage(dir->playFile(pickedEntry, "image"));
            break;
        }

    } else {
        switch (id) {
        case Door:
            if (dir->cdUp()) {
                hoveringId = -1;
                leavingDoor = id;
                curRoom->clearBack();
                startAnimation(Leaving1);
            }
            break;

        case LeftArrow:
            dir->prevPage();
            curRoom->loadFront(dir);
            update();
            break;

        case RightArrow:
            dir->nextPage();
            curRoom->loadFront(dir);
            update();
            break;

        case Image:
            openFile(dir->getPlayingFile("image"));
            break;

        case ImagePrevBtn:
            curRoom->setImage(dir->playPrev("image"));
            break;

        case ImageNextBtn:
            curRoom->setImage(dir->playNext("image"));
            break;
        }
    }
}

void View::openEntry(int index)
{
    if (index < dir->countDir()) {
        hoverLeave();
        dir->cd(index);
        curRoom->loadBack(dir);
        enteringDir = index;
        startAnimation(Entering1);
    } else
        openFile(dir->absoluteFilePath(index));
}

void View::mousePressEvent(QMouseEvent *event)
{
    if (animStage != NoAnim || event->button() != Qt::LeftButton) return;
    int obj = objectIdForPoint(event->pos());

    if (obj >= 0 && obj < dir->count()) {
        pickedEntry = obj;
        curRoom->pickEntry(obj);
        QMatrix4x4 mvp = calcMvp(camera(), size());
        pickedDepth = (mvp * curRoom->getEntryPos(pickedEntry)).z();
        pickedPos = mvp.inverted() * extendTo3D(event->pos(), pickedDepth);
        deltaPos = QVector3D(0, 0, 0);
        isNear = true;

    } else if (obj == -1) {
        roamStartCenter = camera()->center();
        roamStartPos = event->pos();
        isRoaming = true;
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    if (animStage != NoAnim || event->button() != Qt::LeftButton) return;

    if (isRoaming) {
        isRoaming = false;
        return;
    }

    if (pickedEntry != -1 && isNear)
        openEntry(pickedEntry);

    else {
        int id = objectIdForPoint(event->pos());
        if (id > dir->count())
            invokeObject(id);
    }

    pickedEntry = -1;
    curRoom->pickEntry(-1);
    update();
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    if (animStage != NoAnim) return;

    if (isRoaming) {
        /* FIXME: moving mouse outside window may cause strange behaviour */
        /* The bug is caused by center() - eye() == (0, y, 0), which is parallel to up vector */
        QVector3D moveVector = (calcMvp(camera(), size()).inverted() * QVector4D(event->pos() - roamStartPos)).toVector3D();
        QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(roamStartCenter, -moveVector), moveVector.length() * 40);
        camera()->setCenter(rotation.rotatedVector(roamStartCenter - QVector3D(0, eyeHeight, 0)) + QVector3D(0, eyeHeight, 0));
        return;
    }

    int obj = objectIdForPoint(event->pos());
    if (obj != hoveringId) {
        hoverLeave();
        hoverEnter(obj);
    }

    if (pickedEntry != -1) {
        /* move picked object */
        deltaPos = calcMvp(camera(), size()).inverted()
            * extendTo3D(event->pos(), pickedDepth)
            - pickedPos;
        update();
        return;
    }
}

void View::hoverEnter(int obj) {
    if (obj == -1) return;
    hoveringId = obj;
    update();
}

void View::hoverLeave() {
    if (hoveringId == -1) return;
    hoveringId = -1;
    updateHudContent();
    update();
}

void View::keyPressEvent(QKeyEvent *event) {
    if (animStage != NoAnim) return;
    switch (event->key()) {
    case Qt::Key_Tab:
        setOption(GLView::ShowPicking, !(options() & GLView::ShowPicking));
        update();
        break;

    case Qt::Key_Left:
        hoverLeave();
        startAnimation(TurningLeft);
        break;

    case Qt::Key_Up:
        hoverLeave();
        dir->prevPage();

        curRoom->loadFront(dir);
        update();
        break;

    case Qt::Key_Right:
        hoverLeave();
        startAnimation(TurningRight);
        break;

    case Qt::Key_Down:
        hoverLeave();
        dir->nextPage();

        curRoom->loadFront(dir);
        update();
        break;

    case Qt::Key_D:
        hoverLeave();
        camera()->setCenter(QVector3D(0, eyeHeight, roomLength / 2));
        camera()->setEye(defaultEye);
        camera()->setNearPlane(roomLength / 2 * 0.015);
        camera()->setFarPlane(roomLength / 2 * 50);
        camera()->setUpVector(QVector3D(0, 1, 0));
        update();
        break;

    case Qt::Key_R:
        hoverLeave();
        camera()->setCenter(defaultCenter);
        camera()->setEye(defaultEye);
        camera()->setNearPlane(roomLength / 2 * 0.015);
        camera()->setFarPlane(roomLength / 2 * 50);
        camera()->setUpVector(QVector3D(0, 1, 0));
        update();
        break;

    case Qt::Key_U:
        if (dir->cdUp()) {
            hoverLeave();
            curRoom->loadFront(dir);
            update();
        }
        break;
    }
}

void View::wheelEvent(QWheelEvent *) { }

inline QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size)
{
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
