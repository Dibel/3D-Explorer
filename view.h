#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>
#include <QtGui/QImage>
#include <QtCore/QDir>

class MeshObject;
class QWidget;
class QPaintDevice;
class QGLSceneNode;
class QGLShaderProgramEffect;

class View : public QGLView {
    Q_OBJECT
public:
    View();

private:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void initializeGL(QGLPainter *painter);
    void paintGL(QGLPainter *painter);

    void initializeBox();
    void sendEnterEvent(QObject *object);
    void sendLeaveEvent(QObject *object);

    QImage paintHud(float x, float y, QString text);
    void drawText(float x, float y,QString text);

    QVector<MeshObject*> objects;
    QGLSceneNode *hudObj;
    QDir dir;

    MeshObject *pickedObj;
    /* original position of picked object */
    QVector3D pickedPos;
    /* clicked position in picked object's local coordinate */
    QVector3D pickedModelPos;
    /* picked object's depth in projected coordinate */
    qreal pickedDepth;

    QMatrix4x4 mvp;

    int shelfSlotNum;

    QObject *enteredObject;

    QGLShaderProgramEffect *hudEffect;

private slots:
    void showFileName(bool hovering);
};

#endif
