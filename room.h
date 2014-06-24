#ifndef ROOM_H
#define ROOM_H

#include <QtGui/QMatrix4x4>

class QGLPainter;
class QGLSceneNode;
class QTextStream;
class Directory;
enum AnimStage : int;

class Room {
public:
    Room(const QString &name);

    void paintFront(QGLPainter *painter, int animObj, qreal animProg) const;
    void paintBack(QGLPainter *painter, AnimStage stage) const;

    void loadFront(Directory *dir);
    void loadBack(Directory *dir);

    inline void switchBackAndFront() { frontPage.swap(backPage); }
    inline void clearBack() { backPage.clear(); }

    void paintPickedEntry(QGLPainter *painter, const QVector3D &deltaPos) const;
    inline void pickEntry(int index) { pickedEntry = index; }

    inline int countSlot() const { return slot.size(); }

    inline QVector3D getOutPos() const { return outPos; }
    inline qreal getOutAngle() const { return outAngle; }

    inline QVector3D getDoorPos() const { return doorPos; }
    inline qreal getDoorAngle() const { return doorAngle; }

    inline QMatrix4x4 getEntryMat(int idx) const { return slot.at(idx); }

private:
    void loadProperty(const QString &property, QTextStream &value);

    QVector<int> frontPage, backPage;
    int pickedEntry = -1;

    QVector3D outPos, doorPos;
    qreal outAngle, doorAngle;

    void loadModel(QTextStream &value);
    void loadEntryModel(QTextStream &value);
    void loadWall(QTextStream &value);

    struct AnimInfo {
        QGLSceneNode *mesh; QVector3D center, axis; qreal maxAngle;
        void draw(QGLPainter *painter, qreal animProg = 0.0) const;
    };

    struct MeshInfo {
        QGLSceneNode *mesh; QMatrix4x4 transform; int id; AnimInfo *anim;
        void draw(QGLPainter *painter, qreal animProg = 0.0) const;
    };

    QVector<MeshInfo> solid;
    QVector<QMatrix4x4> slot;

    void setFloorAndCeil();
    QGLSceneNode *floor, *ceil;

    QVector<QGLSceneNode*> entryModel;
    AnimInfo dirAnim;

    static void paintMesh(QGLPainter *painter,
            QGLSceneNode *mesh, const QMatrix4x4 &trans, int id,
            const AnimInfo *anim = NULL, qreal animProg = 0.0);
};

#endif
