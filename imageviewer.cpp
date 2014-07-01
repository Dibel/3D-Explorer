#include "imageviewer.h"
#include "common.h"
#include "directory.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLPainter>

const QString ImageViewer::defaultImage = ":/data/default.png";

ImageViewer::ImageViewer(int w, int h)
{
    // the body
    QGLBuilder builder;
    builder.newSection(QGL::Faceted);
    builder.addPane(QSizeF(w, h));
    body = builder.finalizedSceneNode();
    body->setMaterial(new QGLMaterial());
    body->setEffect(QGL::FlatReplaceTexture2D);

    // prev button
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

    // next button
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

void ImageViewer::setFile(const QString &fileName)
{
    QGLTexture2D *tex = body->material()->texture();
    if (tex) {
        tex->release();
        tex->deleteLater();
    }
    tex = new QGLTexture2D;
    tex->setImage(QImage(fileName.isEmpty() ? defaultImage : fileName));
    body->material()->setTexture(tex);
}

void ImageViewer::draw(QGLPainter *painter)
{
    painter->modelViewMatrix().push();
    painter->modelViewMatrix() *= trans;

    int prevPickId = painter->objectPickId();
    painter->setObjectPickId(Image);

    body->draw(painter);

    if (painter->isPicking() || hoveringId >= Image) {
        QColor color = painter->color();
        painter->setColor(QColor(Qt::white));

        painter->setObjectPickId(ImagePrevBtn);
        if (hoveringId == ImagePrevBtn)
            hoveringPickColor = painter->pickColor();
        prevBtn->draw(painter);

        painter->setObjectPickId(ImageNextBtn);
        if (hoveringId == ImageNextBtn)
            hoveringPickColor = painter->pickColor();
        nextBtn->draw(painter);

        painter->setColor(color);
    }

    painter->setObjectPickId(prevPickId);
    painter->modelViewMatrix().pop();
}
