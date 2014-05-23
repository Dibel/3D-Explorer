#include "imageobject.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLPainter>
#include <Qt3D/QGLShaderProgramEffect>
#include <Qt3D/QGLTexture2D>
#include <Qt3D/QGLView>

ImageObject::ImageObject(int width, int height, QGLView *view, Type type)
    : PickObject(view, type == Normal ? StartImageId : -2), type(type)
{
    QGLBuilder builder;
    builder.newSection(QGL::Faceted);
    builder.addPane(QSizeF(width, height));
    node = builder.finalizedSceneNode();

    node->setMaterial(new QGLMaterial());
    if (type != Outline)
        node->setEffect(QGL::FlatReplaceTexture2D);
    else {
        outlineEffect = new QGLShaderProgramEffect();
        outlineEffect->setVertexShaderFromFile(":/shader/outline.vsh");
        outlineEffect->setFragmentShaderFromFile(":/shader/outline.fsh");
        node->setUserEffect(outlineEffect);
    }
}

ImageObject::~ImageObject() {
    node->material()->texture()->deleteLater();
    node->material()->deleteLater();
    node->deleteLater();
}

QImage ImageObject::getImage() {
    return image;
}

void ImageObject::setImage(const QImage &newImage) {
    image = newImage;
    QGLTexture2D *tex = node->material()->texture();
    if (tex) {
        tex->release();
        tex->deleteLater();
    }
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

void ImageObject::draw(QGLPainter *painter) {
    if (painter->isPicking() && objectId() == -2) return;

    if (type == Hud) {
        painter->modelViewMatrix().push();
        painter->modelViewMatrix().setToIdentity();
        painter->projectionMatrix().push();
        painter->projectionMatrix().setToIdentity();

        glEnable(GL_BLEND);
        node->draw(painter);
        glDisable(GL_BLEND);

        painter->modelViewMatrix().pop();
        painter->projectionMatrix().pop();

    } else if (type == Outline) {
        glEnable(GL_BLEND);
        outlineEffect->setActive(painter, true);
        node->draw(painter);
        glDisable(GL_BLEND);

    } else if (painter->isPicking()) {
        int prevPickId = painter->objectPickId();
        painter->setObjectPickId(objectId());
        node->draw(painter);
        painter->setObjectPickId(prevPickId);
    } else {
        node->draw(painter);
    }
}
