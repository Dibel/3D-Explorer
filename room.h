#ifndef ROOM_H
#define ROOM_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QVector3D>
#include <QtGui/QMatrix4x4>

class QGLAbstractScene;
class QGLMaterial;
class QGLPainter;
class QGLSceneNode;
class GLView;
class QTextStream;
class Directory;

class Room {
public:
    Room(const QString &name, GLView *view, void *config);

    void setDir(Directory *dir_) { dir = dir_; }

    void paintFront(QGLPainter *painter, int animObj, qreal animProg);
    void paintBack(QGLPainter *painter, int stage);
    void paintPickedEntry(QGLPainter *painter, const QVector3D &deltaPos);

    void loadDir(Directory *dir, bool back = false);
    void pushToFront();
    void clearBack();

    inline int getSlotNum() const { return slotNum; }
    inline QVector3D getOutPos() const { return outPos; }
    inline qreal getOutAngle() const { return outAngle; }
    inline QVector3D getDoorPos() const { return doorPos; }
    inline qreal getDoorAngle() const { return doorAngle; }

    QVector3D getEntryPos(int i) const;

    inline void pickEntry(int index) { pickedEntry = index; }

private:
    void loadProperty(const QString &property, QTextStream &value);

    void setFloorAndCeil();
    void loadModel(QTextStream &value);
    void loadWall(QTextStream &value);
    void loadContainer(const QString &name, const QVector3D &basePos);

    struct AnimInfo {
        QGLSceneNode *mesh; QVector3D center, axis; qreal maxAngle;
        void draw(QGLPainter *painter, qreal animProg = 0.0) const;
    };

    struct MeshInfo {
        QGLSceneNode *mesh; QMatrix4x4 transform; int id; AnimInfo *anim;
        void draw(QGLPainter *painter, qreal animProg = 0.0) const;
    };

    QVector<MeshInfo> solid;

    QVector<QVector3D> slotPos;
    QStringList frontPage, backPage;

    QGLSceneNode *floor, *ceil;

    QGLSceneNode *dirSolidModel;
    QGLSceneNode *dirAnimModel;
    AnimInfo dirAnim;
    QHash<QString, QGLSceneNode*> fileModel;

    int slotNum = 0;
    int entryNum;
    int backEntryNum;

    int pickedEntry = -1;

    QVector3D outPos, doorPos;
    qreal outAngle, doorAngle;

    Directory *dir;
};

#endif
