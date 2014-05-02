#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

#include <QtCore/QObject>
#include <QtGui/QImage>

class QGLPainter;
class QGLSceneNode;
class QGLView;

class ImageObject : public QObject {
    Q_OBJECT
public:
    enum Type { Common, Hud };
    ImageObject(int width, int height, Type type = Common);
    ~ImageObject();

    void setImage(const QImage &image);
    void setImage(const QString &fileName);
    void setPosition(const QVector3D &pos);

    void regist(QGLView *view, int id);
    void draw(QGLPainter *painter);

private:
    QGLSceneNode *node;
    QGLView *view;
    int pickId;
    Type type;
};

#endif