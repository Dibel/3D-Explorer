#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>
#include <QtGui/QImage>
#include <QtCore/QDir>

class MeshObject;
class QPaintDevice;
class QGLSceneNode;
class QGLShaderProgramEffect;
class QGLTexture2D;

class View : public QGLView {
    Q_OBJECT
public:
    View(int width = 800, int height = 600);
    ~View();

protected:
    void initializeGL(QGLPainter *painter);
    void paintGL(QGLPainter *painter);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *);
    void resizeEvent(QResizeEvent *);

private:
    void initializeHud();
    QImage paintHud(float x, float y, QString text);
    void drawText(float x, float y,QString text);
    void initializeBox();
    void updateDir();
    void hoverEnter(MeshObject *object);
    void hoverLeave();

    MeshObject *meshObjectAt(const QPoint &pos);

    QVector<MeshObject*> background;
    QVector<MeshObject*> boxes;
    QGLSceneNode *picture;
    QGLSceneNode *hudObject;
    QGLShaderProgramEffect *hudEffect;

    QDir dir;
    int entryCnt;
    int dirEntryCnt;
    int slotCnt;

    QMatrix4x4 mvp;

    MeshObject *pickedObject;
    /* original position of picked object */
    QVector3D pickedPos;
    /* clicked position in picked object's local coordinate */
    QVector3D pickedModelPos;
    /* picked object's depth in projected coordinate */
    qreal pickedDepth;

    MeshObject *enteredObject;

};

#endif
