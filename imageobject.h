#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

#include "pickobject.h"
#include <QtGui/QImage>

class QGLPainter;
class QGLSceneNode;
class QGLShaderProgramEffect;
class QGLView;

class ImageObject : public PickObject {
    Q_OBJECT
public:
    enum Type { Normal, Background, Hud, Outline };

    ImageObject(int width, int height, QGLView *view, Type type = Normal);
    ~ImageObject();

    QImage getImage();
    void setImage(const QImage &image);
    void setImage(const QString &fileName);
    void setPosition(const QVector3D &pos);

    void draw(QGLPainter *painter);

private:
    QGLSceneNode *node;
    QGLView *view;
    Type type;
    QImage image;

    QGLShaderProgramEffect *outlineEffect;
};

#endif
