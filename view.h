#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>
#include <QtGui/QImage>
#include <QtCore/QDir>

class MeshObject;
class QPaintDevice;
class QGLSceneNode;
class QGLShaderProgramEffect;

class View : public QGLView {
    Q_OBJECT
public:
    View();

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
    void updateBoxes();
    void hoverEnter(MeshObject *object);
    void hoverLeave();

    MeshObject *meshObjectAt(const QPoint &pos);

    QVector<MeshObject*> boxes;
    MeshObject *background;
    QGLSceneNode *hudObj;
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

    QGLSceneNode *picture;
};

#endif
