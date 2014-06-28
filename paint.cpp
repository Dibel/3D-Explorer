#include "view.h"
#include "common.h"
#include "directory.h"
#include "outlinepainter.h"
#include "room.h"
#include <Qt3D/QGLFramebufferObjectSurface>
#include <Qt3D/QGLShaderProgramEffect>
#include <Qt3D/QGLSceneNode>
#include <QtGui/QOpenGLShaderProgram>

inline QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);

inline QVector3D rotateCcw(QVector3D vec, qreal angle)
{
    return QQuaternion::fromAxisAndAngle(0, 1, 0, angle).rotatedVector(vec);
}

void View::paintGL(QGLPainter *painter)
{
    if (painter->isPicking()) {
        curRoom->paintFront(painter);
        return;
    }

    painter->removeLight(0);
    painter->addLight(light);

    painter->setUserEffect(phongEffect);
    phongEffect->program()->setUniformValue("ambientColor", 0.2f, 0.2f, 0.2f, 1.0f);
    phongEffect->program()->setUniformValue("diffuseColor", 1.0f, 1.0f, 1.0f, 1.0f);
    phongEffect->program()->setUniformValue("specularColor", 1.0f, 1.0f, 1.0f, 1.0f);

    updateCamera();
    paintCurrentRoom(painter);
    if (animStage > NoAnim && animStage < Leaving3)
        paintNextRoom(painter);

    if (enteringDir != -1 || leavingDoor != -1) return;
    // else no animation

    if (hoveringId != -1)
        paintOutline(painter);

    if (pickedEntry != -1) {
        if (deltaPos.length() > 1) isNear = false;
        if (!isNear) glClear(GL_DEPTH_BUFFER_BIT);
        curRoom->paintPickedEntry(painter, deltaPos);
    }

    paintHud(painter);
}

void View::updateCamera()
{
    switch (animStage) {
    case Leaving3:
        {
            qreal angle = curRoom->getOutAngle();
            if (curRoom->getOutAngle() > 180)
                angle = angle + (360 - angle) * animProg;
            else
                angle = angle * (1 - animProg);
            camera()->setCenter(rotateCcw(defaultCenter, angle));
        }
        break;

    case TurningLeft:
        camera()->setCenter(rotateCcw(startCenter, 90.0 * animProg));
        break;
    
    case TurningRight:
        camera()->setCenter(rotateCcw(startCenter, -90.0 * animProg));
        break;

    default:
        break;
    }
}

void View::paintCurrentRoom(QGLPainter *painter)
{
    int id;
    qreal prog;

    // door and chest
    switch (animStage) {
    case Entering1:
        id = enteringDir;
        prog = animProg;
        break;

    case Entering2:
        id = enteringDir;
        prog = 1.0;
        break;

    case Leaving1:
        id = leavingDoor;
        prog = qMin(animProg * 2, 1.0);
        break;

    default:
        id = leavingDoor;
        prog = 1.0;
    }

    curRoom->paintFront(painter, id, prog);
}

void View::paintNextRoom(QGLPainter *painter)
{
    painter->removeLight(lightId);

    // first slow, then quick, then slow
    qreal t = animProg;
    if (t > 0.5) {
        t = 2 - t * 2;
        t = (2 - t * t) * 0.5;
    } else t = t * t * 2;

    camera()->setCenter(startCenter + t * deltaCenter);
    camera()->setEye(startEye + t * deltaEye);
    camera()->setUpVector(startUp + t * deltaUp);

    painter->modelViewMatrix().push();
    if (enteringDir != -1) {
        // paint inside
        painter->modelViewMatrix() *= curRoom->getEntryMat(enteringDir);
        painter->modelViewMatrix().scale(boxScale * 0.999);
    } else {
        // paint outside
        painter->modelViewMatrix().rotate(QQuaternion::fromAxisAndAngle(0, 1, 0, curRoom->getDoorAngle() - curRoom->getOutAngle()));
        painter->modelViewMatrix().scale(1.0 / boxScale);
        painter->modelViewMatrix().translate(-curRoom->getOutPos() - QVector3D(0, 0.1, 0));
    }

    int tmpLightId = painter->addLight(light);

    curRoom->paintBack(painter, animStage);

    painter->modelViewMatrix().pop();

    painter->removeLight(tmpLightId);
    lightId = painter->addLight(light);
}

void View::paintOutline(QGLPainter *painter)
{
    // FIXME: unable to reuse pick buffer of GLView

    if (!fbo) {
        fbo = new QOpenGLFramebufferObject(size(), QOpenGLFramebufferObject::CombinedDepthStencil);
        surface = new QGLFramebufferObjectSurface(fbo);
    }

    painter->pushSurface(surface);
    painter->setPicking(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    curRoom->paintFront(painter);

    painter->setPicking(false);
    painter->popSurface();

    outline->draw(painter, fbo->texture());
}

void View::paintHud(QGLPainter *painter)
{
    if (hoveringId != -1 && hoveringId < dir->count()) {
        QVector3D pos = calcMvp(camera(), size()) * curRoom->getEntryPos(hoveringId);
        updateHudContent(pos.x(), pos.y(), dir->entry(hoveringId));
    }

    painter->modelViewMatrix().push();
    painter->modelViewMatrix().setToIdentity();
    painter->projectionMatrix().push();
    painter->projectionMatrix().setToIdentity();

    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    hud->draw(painter);
    glDisable(GL_BLEND);

    painter->modelViewMatrix().pop();
    painter->projectionMatrix().pop();
}

void View::updateHudContent(qreal x, qreal y, QString text)
{
    QImage image(width(), height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = painter.font();
    font.setPointSize(18);
    font.setBold(1);

    QPainterPath path;
    if (!text.isEmpty()) path.addText(x, y, font, text);
    path.addText(0, 20, font, dir->absolutePath());
    painter.setBrush(QColor(Qt::white));
    painter.setPen(QColor(Qt::black));
    painter.drawPath(path);

    QGLTexture2D *tex = hud->material()->texture();
    if (tex) {
        tex->release();
        tex->deleteLater();
    }
    tex = new QGLTexture2D();
    tex->setImage(image);
    hud->material()->setTexture(tex);
}

void View::setupLight()
{
    phongEffect = new QGLShaderProgramEffect();
    phongEffect->setVertexShaderFromFile(":/shader/phong.vsh");
    phongEffect->setFragmentShaderFromFile(":/shader/phong.fsh");

    boxEffect = new QGLShaderProgramEffect();
    boxEffect->setVertexShaderFromFile(":/shader/box.vsh");
    boxEffect->setFragmentShaderFromFile(":/shader/box.fsh");

    light = new QGLLightParameters(this);
    light->setPosition(QVector3D(0, roomHeight * 0.5, 0));
    light->setAmbientColor(QColor(120, 120, 120));
}

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
