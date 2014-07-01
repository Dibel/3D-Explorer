#include "pdfviewer.h"
#include "common.h"
#include "poppler-qt5.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLPainter>

#include <QtCore/QDebug>

PdfViewer::PdfViewer(int w, int h)
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

void PdfViewer::setFile(const QString &fileName)
{
    Poppler::Document *file = Poppler::Document::load(fileName);
    if (!file || file->isLocked()) {
        qDebug() << "Failed to open" << fileName;
        delete file;
        return;
    }

    Poppler::Page *page = file->page(0);
    if (!page) {
        qDebug() << "Failed to load page";
        return;
    }

    QGLTexture2D *tex = body->material()->texture();
    if (tex) {
        tex->release();
        tex->deleteLater();
    }
    tex = new QGLTexture2D;
    tex->setImage(page->renderToImage());
    body->material()->setTexture(tex);
}

void PdfViewer::draw(QGLPainter *painter)
{
    if (painter->isPicking()) return;

    painter->modelViewMatrix().push();
    painter->modelViewMatrix() *= trans;

    body->draw(painter);

    painter->modelViewMatrix().pop();
}
