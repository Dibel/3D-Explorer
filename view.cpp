#include "view.h"
#include "common.h"
#include "directory.h"
#include "outlinepainter.h"
#include "imageviewer.h"
#include "room.h"
#include <Qt3D/QGLBuilder>

View::View(int width, int height, const QSurfaceFormat &format) : GLView(format)
{
    resize(width, height);

    windowWidth = width;
    windowHeight = height;

    curRoom = rooms["room1"];
    dir = new Directory(curRoom->countSlot());

    defaultCenter = QVector3D(0, eyeHeight, -roomLength / 2);
    defaultEye = QVector3D(0, eyeHeight, 0);

    camera()->setCenter(defaultCenter);
    camera()->setEye(defaultEye);

    camera()->setNearPlane(roomLength / 2 * 0.015);
    camera()->setFarPlane(roomWidth * 50);

    setupLight();
    setupAnimation();

    // HUD
    QGLBuilder builder;
    builder.newSection(QGL::Faceted);
    builder.addPane(QSizeF(2, 2));
    hud = builder.finalizedSceneNode();
    hud->setMaterial(new QGLMaterial);
    hud->setEffect(QGL::FlatReplaceTexture2D);

    /* outline */
    outline = new OutlinePainter;

    curRoom->loadFront(dir);
    updateHudContent();
    update();
}

void View::initializeGL(QGLPainter *painter)
{
    lightId = painter->addLight(light);
}

void View::resizeEvent(QResizeEvent *)
{
    updateHudContent();
    update();
}
