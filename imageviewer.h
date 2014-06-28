#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QtGui/QImage>
#include <QtGui/QMatrix4x4>

class QGLPainter;
class QGLSceneNode;

/**
 * \brief Image previewer class
 *
 * Load image from file and represent it as a pane in 3D space.
 *
 * When the ImageViewer is hovered, display two buttons
 * for user to view other images.
 */

class ImageViewer {
public:
    /// Create a ImageViewer of specific size.
    ImageViewer(int width, int height);

    /// Update and draw the image.
    void draw(QGLPainter *painter);

    /// Load an image from file and display it.
    void setFile(const QString &fileName);

    /// TODO: manage position in Room
    void setPosition(const QVector3D &pos)
    {
        trans.translate(pos);
    }

private:
    QMatrix4x4 trans;
    QGLSceneNode *body, *prevBtn, *nextBtn;

    static const QString defaultImage;
};

#endif
