#ifndef ROOM_H
#define ROOM_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QVector3D>

class QGLMaterial;
class QGLPainter;
class QGLSceneNode;
class QGLView;
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

    QGLSceneNode *dirModel;
    QGLSceneNode *fileModel;

    static Room *loadRoom(const QString &fileName, const QHash<QString, QGLMaterial*> &palette, QGLView *view);
    void paintCurRoom(QGLPainter *painter, MeshObject *animObj, qreal animProg);
    void paintNextRoom(QGLPainter *painter, int stage);
};

#endif
