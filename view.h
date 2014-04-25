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

    void initializeGL(QGLPainter *painter);
    void paintGL(QGLPainter *painter);

    QVector<MeshObject*> objects;

    QDir dir;

    int shelfSlotNum;
};

#endif
