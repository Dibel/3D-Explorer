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
class GLView;
class QTextStream;
//class Config;
class MeshObject;
class Directory;

class Room {
public:
    Room(const QString &name, GLView *view, void *config);

    void paintFront(QGLPainter *painter, int animObj, qreal animProg);
    void paintBack(QGLPainter *painter, int stage);
    void paintPickedEntry(QGLPainter *painter, const QVector3D &deltaPos);

    void loadDir(Directory *dir, bool back = false);
    void pushToFront();
    void clearBack();

    inline int getSlotNum() const { return slotNum; }
    inline QVector3D getOutPos() const { return outPos; }
    inline qreal getOutAngle() const { return outAngle; }

    QVector3D getEntryPos(int i) const;
    QVector3D getDoorPos() const;
    qreal getDoorAngle() const;

    inline void pickEntry(int index) { pickedEntry = index; }

private:
    void loadProperty(const QString &property, QTextStream &value);

    void setFloorAndCeil();
    void loadModel(QTextStream &value);
    void loadWall(QTextStream &value);
    void loadContainer(const QString &name, MeshObject *mesh);

    QVector<MeshObject*> solid;
    QVector<MeshObject*> entry;
    QVector<MeshObject*> backEntry;
    MeshObject *floor;
    MeshObject *ceil;

    MeshObject *door;

    QGLSceneNode *dirSolidModel;
    QGLSceneNode *dirAnimModel;
    QHash<QString, QGLSceneNode*> fileModel;

    int slotNum;
    int entryNum;
    int backEntryNum;

    int pickedEntry = -1;

    QVector3D outPos;
    qreal outAngle;

    //QGLView *view;
    //Config *config;
};

#endif
