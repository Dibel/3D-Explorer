#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>
#include <QtCore/QDir>

class MeshObject;

class View : public QGLView {
    Q_OBJECT
public:
    View();

private:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void initializeGL(QGLPainter *painter);
    void paintGL(QGLPainter *painter);

    QVector<MeshObject*> objects;
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
private slots:
	void showFileName(bool hovering);
};

#endif
