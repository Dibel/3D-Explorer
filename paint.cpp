#include "view.h"
#include "meshobject.h"
#include "imageobject.h"
#include "directory.h"
#include <QtGui/QOpenGLShaderProgram>
#include <Qt3D/QGLFramebufferObjectSurface>
#include <Qt3D/QGLShaderProgramEffect>

static QMatrix4x4 calcMvp(const QGLCamera *camera, const QSize &size);

void View::paintGL(QGLPainter *painter) {
    Q_ASSERT(picture != NULL);
    Q_ASSERT(hudObject != NULL);

    mvp = calcMvp(camera(), size());

    painter->addLight(light);
    //painter->addLight(light2);
    painter->setUserEffect(phongEffect);
    phongEffect->program()->setUniformValue("ambientColor", 0.2f, 0.2f, 0.2f, 1.0f);
    phongEffect->program()->setUniformValue("diffuseColor", 1.0f, 1.0f, 1.0f, 1.0f);
    phongEffect->program()->setUniformValue("specularColor", 1.0f, 1.0f, 1.0f, 1.0f);
    for (auto obj : staticMeshes) obj->draw(painter);
    floor->draw(painter);
    ceil->draw(painter);
    for (auto obj : boxes) 
        if (obj != enteringDir && obj != pickedObject)
            obj->draw(painter);
    picture->draw(painter);

    if (enteringDir || isLeavingDir) {
        if (painter->isPicking()) return;

        //QVector3D lPos = light->position();
        //light->setPosition(lPos + enteringDir->position());
        //painter->addLight(light);
        //phongEffect->setActive(painter, true);

        qreal t = animProg;
        if (isLeavingDir) t = 1.0 - t;
        if (t > 0.5) {
            t = 2 - t * 2;
            t = (2 - t * t) * 0.5;
        } else t = t * t * 2;
        camera()->setCenter(startCenter + t * deltaCenter);
        camera()->setEye(startEye + t * deltaEye);
        camera()->setUpVector(startUp + t * deltaUp);

        painter->modelViewMatrix().push();
        if (enteringDir)
            painter->modelViewMatrix().translate(enteringDir->position());
        else
            painter->modelViewMatrix().translate(boxes[0]->position());
        painter->modelViewMatrix().scale(boxScale * 0.99);

        for (auto obj : staticMeshes) obj->draw(painter);
        for (auto obj : backBoxes) obj->draw(painter);
        backPicture->draw(painter);

        painter->modelViewMatrix().translate(0, 0.1, 0);
        floor->draw(painter);
        painter->modelViewMatrix().pop();

        //light->setPosition(lPos);
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    if (pickedObject)
        pickedObject->draw(painter);

    if (enteredObject && enteredObject->pickType() == MeshObject::Normal) outline->draw(painter);
    glClear(GL_DEPTH_BUFFER_BIT);
    if (!(enteringDir || isLeavingDir)) hudObject->draw(painter);
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

    if (text.isEmpty() && isShowingFileName)
        for (int i = 0; i < dir->count(); ++i) {
            QVector3D pos = mvp * boxes[i]->position();
            QRect rect(pos.x() - 30, pos.y(), 60, 30);
            QString text = boxes[i]->objectName();
            painter.drawText(rect, Qt::AlignHCenter | Qt::TextWrapAnywhere,
                    boxes[i]->objectName());
        }
    
    hudObject->setImage(image);
}

class Surface : public QGLAbstractSurface {
public:
    Surface(QGLView *view, QOpenGLFramebufferObject *fbo, const QSize &areaSize) :
        QGLAbstractSurface(504), m_view(view), m_fbo(fbo),
        m_viewportGL(QPoint(0, 0), areaSize) { }

    QPaintDevice *device() const;
    bool activate(QGLAbstractSurface *) { if (m_fbo) m_fbo->bind(); return true; }
    void deactivate(QGLAbstractSurface *) { if (m_fbo) m_fbo->release(); }
    QRect viewportGL() const { return m_viewportGL; }
private:
    QGLView *m_view;
    QOpenGLFramebufferObject *m_fbo;
    QRect m_viewportGL;
};

void View::paintOutline(MeshObject *obj) {
    QVector3D pos = mvp * obj->position();
    paintHud(pos.x(), pos.y(), obj->objectName());

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

    enteredObject->ignoreMuting(true);
    PickObject::muteObjectId(true);

    paintGL(&painter);

    enteredObject->ignoreMuting(false);
    PickObject::muteObjectId(false);

    painter.setPicking(false);
    painter.popSurface();

    /* FIXME: use texture id */
    outline->setImage(fbo->toImage());
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
