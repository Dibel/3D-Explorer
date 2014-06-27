#include "imageviewer.h"
#include "common.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLPainter>

const QString ImageViewer::defaultImage = ":/data/default.png";

ImageViewer::ImageViewer(int w, int h)
{
    QGLBuilder builder;
    builder.newSection(QGL::Faceted);
    builder.addPane(QSizeF(w, h));
    body = builder.finalizedSceneNode();
    body->setMaterial(new QGLMaterial());
    body->setEffect(QGL::FlatReplaceTexture2D);

    QVector3DArray vertices;
    vertices.append(-w * 0.4, 0, 0.01);
    vertices.append(-w * 0.3, -h * 0.1, 0.01);
    vertices.append(-w * 0.3, h * 0.1, 0.01);
    QGeometryData triangle;
    triangle.appendVertexArray(vertices);
    QGLBuilder prevBuilder;
    prevBuilder.newSection(QGL::Faceted);
    prevBuilder.addTriangles(triangle);
    prevBtn = prevBuilder.finalizedSceneNode();
    prevBtn->setEffect(QGL::FlatColor);

    vertices.clear();
    vertices.append(w * 0.4, 0, 0);
    vertices.append(w * 0.3, h * 0.1, 0.01);
    vertices.append(w * 0.3, -h * 0.1, 0.01);
    triangle.clear();
    triangle.appendVertexArray(vertices);
    QGLBuilder nextBuilder;
    nextBuilder.newSection(QGL::Faceted);
    nextBuilder.addTriangles(triangle);
    nextBtn = nextBuilder.finalizedSceneNode();
    nextBtn->setEffect(QGL::FlatColor);
}

void ImageViewer::setImage(const QImage &newImage)
{
    image = newImage;
    QGLTexture2D *tex = body->material()->texture();
    if (tex) {
        tex->release();
        tex->deleteLater();
    }
    tex = new QGLTexture2D;
    tex->setImage(image);
    body->material()->setTexture(tex);
}

void ImageViewer::draw(QGLPainter *painter, bool drawBtn)
{
    painter->modelViewMatrix().push();
    painter->modelViewMatrix() *= trans;

    int prevPickId = painter->objectPickId();
    //if (paintingOutline == -1 || paintingOutline == Image)
    painter->setObjectPickId(Image);

    body->draw(painter);

    /* FIXME: update outline algorithm to distinguish buttons */
    if (drawBtn) {
        QColor color = painter->color();
        painter->setColor(QColor(Qt::white));
        painter->setObjectPickId(ImagePrevBtn);
        prevBtn->draw(painter);
        painter->setObjectPickId(ImageNextBtn);
        nextBtn->draw(painter);
        painter->setColor(color);
    }

    painter->setObjectPickId(prevPickId);
    painter->modelViewMatrix().pop();
}
