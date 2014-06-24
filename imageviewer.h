#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QtGui/QImage>
#include <QtGui/QMatrix4x4>

class QGLPainter;
class QGLSceneNode;

class ImageViewer {
public:
    ImageViewer(int width, int height);

    inline QImage getImage() { return image; }

    void setImage(const QImage &image);

    inline void setImage(const QString &fileName)
    {
        setImage(QImage(fileName.isEmpty() ? defaultImage : fileName));
    }

    void setPosition(const QVector3D &pos)
    {
        trans.translate(pos);
    }

    void draw(QGLPainter *painter, bool drawButtons = false);

private:
    QMatrix4x4 trans;
    QGLSceneNode *body, *prevBtn, *nextBtn;
    QImage image;

    static const QString defaultImage;
};

#endif
