#ifndef ROOM_H
#define ROOM_H

#include <QtGui/QMatrix4x4>

class QGLPainter;
class QGLSceneNode;
class QTextStream;
class Directory;
class ImageViewer;
enum AnimStage : int;

/**
 * \brief A displayable room representing a directory.
 *
 * The room contains a set of solid models and a list of file system entries,
 * as well as floor, ceil, and walls.
 *
 * The solid models may or may not be interactive, but all models cannot be
 * moved. Models of certain types (door at present) can play animation during
 * some action.
 *
 * The entries are kept by Room because they will be swept out at once by
 * Directory when path changed, but must be available for painting until
 * animation finished.
 * Unlike Directory class, entries in Room only contain file type informtion.
 *
 * The Room is highly flexible via a configuration file.
 */

class Room {
public:
    /// Load a room from config file.
    /// See examples in config directory for furthur information.
    Room(const QString &fileName);

    /// Paint the room as current one (i.e. the normal room) to painter.
    /// The \p animObj indicates the currently activated animation object,
    /// and \p animProg defines the progress of animation (0.0 ~ 1.0)
    void paintFront(QGLPainter *painter, int animObj = -1, qreal animProg = 0.0) const;

    /// Paint the room as next one to painter (during animation).
    /// The room is painted as it is except adjustment of floor and ceil,
    /// global geometry change should be applied outside.
    void paintBack(QGLPainter *painter, AnimStage stage) const;

    /// Load entries for paintFont from @p dir.
    void loadFront(Directory *dir);
    /// Load entries for paintBack from @p dir.
    void loadBack(Directory *dir);

    /// Push the back entries to front. Typically Called on end of animation.
    void switchBackAndFront();

    /// Clear all back entries.
    inline void clearBack() { backPage.clear(); }

    /// Pick up an entry.
    /// The picked entry should overlay any other items thus will not be
    /// painted like common entires.
    inline void pickEntry(int index) { pickedEntry = index; }

    /// Paint the picked entry.
    /// The position is moved by &deltaPos because of user action.
    void paintPickedEntry(QGLPainter *painter, const QVector3D &deltaPos) const;

    /// Return the max capacity of entries.
    inline int countSlot() const { return slot.size(); }

    /// Return the starting eye position when come from subdirectories.
    inline QVector3D getOutPos() const { return outPos; }
    /// Return the look-at angle when come from subdirectories.
    inline qreal getOutAngle() const { return outAngle; }

    /// Return the position of door in this room.
    inline QVector3D getDoorPos() const { return doorPos; }
    /// Return the direction of door in this room.
    inline qreal getDoorAngle() const { return doorAngle; }

    /// Return the model-view matrix of entry @p idx.
    inline QMatrix4x4 getEntryMat(int idx) const { return slot.at(idx); }

    /// Return the position of entry @p idx.
    inline QVector3D getEntryPos(int idx) const { return slot.at(idx) * QVector3D(); }

    /// Return the position of solid model with @p id.
    QVector3D getSolidPos(int id) const;

    /// Preview the image in this room.
    void setImage(const QString &fileName);

private:
    void loadProperty(const QString &property, QTextStream &value);

    // helper functions for loadProperty
    void loadModel(QTextStream &value);
    void loadEntryModel(QTextStream &value);
    void loadWall(QTextStream &value);

    // set up floor and ceil mesh
    void setFloorAndCeil();
    QGLSceneNode *floor, *ceil;

    // one-time initialized properties
    QVector3D outPos, doorPos;
    qreal outAngle, doorAngle;

    // variable states
    QVector<int> frontPage, backPage;
    int pickedEntry = -1;

    struct AnimInfo {
        QGLSceneNode *mesh; QVector3D center, axis; qreal maxAngle;
        void draw(QGLPainter *painter, qreal animProg = 0.0) const;
    };

    struct MeshInfo {
        QGLSceneNode *mesh; QMatrix4x4 transform; int id; AnimInfo *anim;
        void draw(QGLPainter *painter, qreal animProg = 0.0) const;
    };

    // room content
    QVector<MeshInfo> solid;
    QVector<QMatrix4x4> slot;

    // models of various file type
    QVector<QGLSceneNode*> entryModel;
    AnimInfo dirAnim;

    ImageViewer *frontImage, *backImage;

    static void paintMesh(QGLPainter *painter,
            QGLSceneNode *mesh, const QMatrix4x4 &trans, int id,
            const AnimInfo *anim = NULL, qreal animProg = 0.0);
};

#endif
