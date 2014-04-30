#include "imageobject.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLPainter>
#include <Qt3D/QGLTexture2D>
#include <Qt3D/QGLView>

ImageObject::ImageObject(int width, int height) {
    QGLBuilder builder;
    builder.addPane(QSizeF(width, height));
    node = builder.finalizedSceneNode();

    node->setMaterial(new QGLMaterial());
    node->setEffect(QGL::FlatDecalTexture2D);
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
    tex = new QGLTexture2D;
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
    if (painter->isPicking()) {
        if (pickId == -1) return;
        int prevPickId = painter->objectPickId();
        painter->setObjectPickId(pickId);
        node->draw(painter);
        painter->setObjectPickId(prevPickId);
    } else {
        node->draw(painter);
    }
}
