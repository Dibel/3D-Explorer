#include "view.h"
#include "meshobject.h"
#include "imageobject.h"
#include "directory.h"
#include "common.h"
#include "room.h"
#include <QtGui/QOpenGLShaderProgram>
#include <Qt3D/QGLFramebufferObjectSurface>
#include <Qt3D/QGLShaderProgramEffect>

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);


void View::paintGL(QGLPainter *painter) {
    Q_ASSERT(picture != NULL);
    Q_ASSERT(hudObject != NULL);
    
    mvp = calcMvp(camera(), size());

    painter->removeLight(0);
    painter->addLight(light);

    painter->setUserEffect(phongEffect);
    phongEffect->program()->setUniformValue("ambientColor", 0.2f, 0.2f, 0.2f, 1.0f);
    phongEffect->program()->setUniformValue("diffuseColor", 1.0f, 1.0f, 1.0f, 1.0f);
    phongEffect->program()->setUniformValue("specularColor", 1.0f, 1.0f, 1.0f, 1.0f);

    if (animStage == Leaving3) {
        qreal t;
        if (curRoom->getOutAngle() > 180)
            t = curRoom->getOutAngle() + (360 - curRoom->getOutAngle()) * animProg;
        else
            t = curRoom->getOutAngle() * (1 - animProg);
        camera()->setCenter(rotateCcw(defaultCenter, t));

    } else if (animStage == TurningLeft) {
        camera()->setCenter(rotateCcw(startCenter, 90.0 * animProg));
    } else if (animStage == TurningRight) {
        camera()->setCenter(rotateCcw(startCenter, -90.0 * animProg));
    }

    qreal t = animProg;
    if (animStage == Leaving1)
        t = t > 0.5 ? 1 : t * 2;
    else
        t = 1;

    if (enteringDir != -1)
        t = animStage == Entering1 ? animProg : 1;

    if (enteringDir != -1)
        curRoom->paintFront(painter, enteringDir, t);
    else
        curRoom->paintFront(painter, leavingDoor, t);

    picture->draw(painter);

    if (animStage > NoAnim && animStage < Leaving3 && !painter->isPicking()) {
        painter->removeLight(lightId);

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
            painter->modelViewMatrix().translate(curRoom->getEntryPos(enteringDir));
            painter->modelViewMatrix().scale(boxScale * 0.999);
        } else {
            painter->modelViewMatrix().rotate(QQuaternion::fromAxisAndAngle(0, 1, 0, curRoom->getDoorAngle() - curRoom->getOutAngle()));
            painter->modelViewMatrix().scale(1.0 / boxScale);
            painter->modelViewMatrix().translate(-curRoom->getOutPos() - QVector3D(0, 0.1, 0));
        }

        int tmpLightId = painter->addLight(light);

        curRoom->paintBack(painter, animStage);

        if (animStage != Leaving1)
            backPicture->draw(painter);

        painter->modelViewMatrix().pop();

        painter->removeLight(tmpLightId);
        lightId = painter->addLight(light);
    }

    if (pickedObject != -1 && !painter->isPicking()) {
        if (deltaPos.length() > 1) isNear = false;
        if (!isNear) glClear(GL_DEPTH_BUFFER_BIT);
        curRoom->paintPickedEntry(painter, deltaPos);
    }

    if (hoveringObject != -1)
        outline->draw(painter);
    glClear(GL_DEPTH_BUFFER_BIT);
    if (!(enteringDir != -1 || leavingDoor != -1)) hudObject->draw(painter);
}

void View::paintHud(qreal x, qreal y, QString text) {
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

    //if (text.isEmpty() && isShowingFileName)
    //    for (int i = 0; i < dir->count(); ++i) {
    //        QVector3D pos = mvp * curRoom->entry[i]->position();
    //        QRect rect(pos.x() - 30, pos.y(), 60, 30);
    //        QString text = curRoom->entry[i]->objectName();
    //        painter.drawText(rect, Qt::AlignHCenter | Qt::TextWrapAnywhere,
    //                curRoom->entry[i]->objectName());
    //    }
    
    hudObject->setImage(image);
}

class Surface : public QGLAbstractSurface {
public:
    Surface(GLView *view, QOpenGLFramebufferObject *fbo, const QSize &areaSize) :
        QGLAbstractSurface(504), m_view(view), m_fbo(fbo),
        m_viewportGL(QPoint(0, 0), areaSize) { }

    QPaintDevice *device() const;
    bool activate(QGLAbstractSurface *) { if (m_fbo) m_fbo->bind(); return true; }
    void deactivate(QGLAbstractSurface *) { if (m_fbo) m_fbo->release(); }
    QRect viewportGL() const { return m_viewportGL; }
private:
    GLView *m_view;
    QOpenGLFramebufferObject *m_fbo;
    QRect m_viewportGL;
};

void View::paintOutline(int obj) {
    if (obj >= 0 && obj < dir->count()) {
        QVector3D pos = mvp * curRoom->getEntryPos(obj);
        paintHud(pos.x(), pos.y(), dir->entry(obj));
    }

    if (!fbo)
        fbo = new QOpenGLFramebufferObject(size(), QOpenGLFramebufferObject::CombinedDepthStencil);
    if (!surface)
        surface = new Surface(this, fbo, size());
    QGLPainter painter(this);
    painter.pushSurface(surface);
    painter.setPicking(true);
    painter.clearPickObjects();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    painter.setEye(QGL::NoEye);
    painter.setCamera(camera());

    PickObject::paintOutline(hoveringObject);

    paintGL(&painter);

    PickObject::paintOutline(-1);

    painter.setPicking(false);
    painter.popSurface();

    /* FIXME: use texture id */
    outline->setImage(fbo->toImage());
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

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size) {
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
