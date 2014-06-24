#include "view.h"
#include "common.h"
#include "directory.h"
#include "imageobject.h"
#include "imageviewer.h"
#include "room.h"

View::View(int width, int height) :
    pickedObject(-1), hoveringObject(-1),
    fbo(NULL), surface(NULL),
    isRoaming(false),
    enteringDir(-1), leavingDoor(-1),
    isShowingFileName(false)
{
    resize(width, height);
}

void View::load()
{
    curRoom = rooms["room1"];
    dir = new Directory;

    dir->setPageSize(curRoom->countSlot());
    //curRoom->setDir(dir);

    defaultCenter = QVector3D(0, eyeHeight, -roomLength / 2);
    defaultEye = QVector3D(0, eyeHeight, 0);

    camera()->setCenter(defaultCenter);
    camera()->setEye(defaultEye);

    camera()->setNearPlane(roomLength / 2 * 0.015);
    camera()->setFarPlane(roomWidth * 50);

    setupLight();
    setupAnimation();
    setupObjects();

    loadDir();
}

View::~View() {
    /* other members should be deleted by QObject system */
    //delete animation;
    //delete dir;
}

void View::setupObjects()
{
    /* picture */
    picture = new ImageViewer(30, 20);
    picture->setPosition(QVector3D(-50, 50, 1 - roomLength / 2));

    backPicture = new ImageViewer(30, 20);
    backPicture->setPosition(QVector3D(-50, 50, 1 - roomLength / 2));

    /* HUD */
    hudObject = new ImageObject(2, 2, this, ImageObject::Hud);

    /* outline */
    outline = new ImageObject(2, 2, this, ImageObject::Outline);
}

void View::loadDir(bool back) {
    if (back) {
        backPicture->setImage(dir->getPlayingFile("image"));
        curRoom->loadBack(dir);
    } else {
        picture->setImage(dir->getPlayingFile("image"));
        curRoom->loadFront(dir);
    }

    //curRoom->loadDir(dir, back);

    paintHud();
    update();
}

void View::debugFunc() {
    qDebug() << "done";
    update();
}

void View::initializeGL(QGLPainter *painter) {
    lightId = painter->addLight(light);
}

void View::resizeEvent(QResizeEvent *) {
    paintHud();
    update();
}
