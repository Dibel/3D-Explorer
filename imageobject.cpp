#include "imageobject.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLPainter>
#include <Qt3D/QGLTexture2D>
#include <Qt3D/QGLView>

ImageObject::ImageObject(int width, int height, Type type)
    : type(type), pickId(-1)
{
    QGLBuilder builder;
    builder.newSection(QGL::Faceted);
    builder.addPane(QSizeF(width, height));
    node = builder.finalizedSceneNode();

    node->setMaterial(new QGLMaterial());
    if (type == Common)
        node->setEffect(QGL::FlatDecalTexture2D);
    else
        node->setEffect(QGL::FlatReplaceTexture2D);
}

ImageObject::~ImageObject() {
    if (view && pickId >= 0)
        view->deregisterObject(pickId);
    node->material()->texture()->deleteLater();
    node->material()->deleteLater();
    node->deleteLater();
}

void ImageObject::setImage(const QImage &image) {
    QGLTexture2D *tex = node->material()->texture();
    if (tex) tex->deleteLater();
    tex = new QGLTexture2D();
    tex->setImage(image);
    node->material()->setTexture(tex);
}

void ImageObject::setImage(const QString &fileName) {
    setImage(QImage(fileName));
}

void ImageObject::setPosition(const QVector3D &pos) {
    node->setPosition(pos);
}

void ImageObject::regist(QGLView *target, int id) {
    view = target;
    pickId = id;
    view->registerObject(id, this);
}

void ImageObject::draw(QGLPainter *painter) {
    if (painter->isPicking() && pickId == -1) return;

    if (type == Hud) {
        Q_ASSERT(pickId == -1);
        Q_ASSERT(node->position() == QVector3D(0, 0, 0));

        painter->modelViewMatrix().push();
        painter->modelViewMatrix().setToIdentity();
        painter->projectionMatrix().push();
        painter->projectionMatrix().setToIdentity();

        glEnable(GL_BLEND);
        node->draw(painter);
        glDisable(GL_BLEND);

        painter->modelViewMatrix().pop();
        painter->projectionMatrix().pop();

    } else if (painter->isPicking()) {
        int prevPickId = painter->objectPickId();
        painter->setObjectPickId(pickId);
        node->draw(painter);
        painter->setObjectPickId(prevPickId);
    } else {
        node->draw(painter);
    }
}
