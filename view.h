#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>

class MeshObject;

class View : public QGLView {
    Q_OBJECT
public:
    View();

private:
    void initializeGL(QGLPainter *painter);
    void paintGL(QGLPainter *painter);

    QVector<MeshObject*> objects;

    int shelfSlotNum;
};

#endif
