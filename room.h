#ifndef ROOM_H
#define ROOM_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QVector3D>

class QGLAbstractScene;
class QGLMaterial;
class QGLPainter;
class QGLSceneNode;
class QGLView;
class QTextStream;
class MeshObject;

class Room {
public:
    Room(const QString &fileName, const QHash<QString, QGLMaterial*> &palette, QGLView *view);

    QVector<MeshObject*> solid;
    QVector<MeshObject*> entry;
    QVector<MeshObject*> backEntry;
    MeshObject *floor;
    MeshObject *ceil;
    QVector3D cdUpPosition;
    qreal cdUpDirection;
    int slotNum;

    const QHash<QString, QGLMaterial*> &palette;
    QGLView *view;

    QGLSceneNode *dirModel;
    QGLSceneNode *dirLidModel;
    QGLSceneNode *fileModel;

    void paintCurRoom(QGLPainter *painter, MeshObject *animObj, qreal animProg);
    void paintNextRoom(QGLPainter *painter, int stage);

private:
    void loadProperty(const QString &property, QTextStream &value);

    void loadDefaultModels();
    void loadModel(QTextStream &value);
    void loadWall(QTextStream &value);
    void loadContainer(const QString &name, MeshObject *mesh);
};

#endif
