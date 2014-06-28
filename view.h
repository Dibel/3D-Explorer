#ifndef SCENE_H
#define SCENE_H

#include "lib/glview.h"

class Directory;
class Hud;
class Room;
class Surface;
class OutlinePainter;
class QGLFramebufferObjectSurface;
class QGLShaderProgramEffect;
class QVariantAnimation;

enum AnimStage : int;

/**
 * \brief The window that show items and reply mouse actions
 *
 * This class represents the main window and maintain most of its properties,
 * including target items, animation stage, lighting parameters, perspective
 * roaming, etc.
 *
 * The class was originally derived from QGLView of Qt3D library, but APIs of 
 * the base class was found not effective enough. So we decided to copy the
 * source code of QGLView and modify it based on our usage. The modified class
 * is named GLView.
 *
 * Definition of member functions are splitted into 4 files due to the
 * complexity of this class.
 */

class View : public GLView {
    Q_OBJECT
public:
    /// Create a window of given size.
    View(int width, int height);

protected:
    /// Virtual function required by QGLView.
    /// Called once for each buffer.
    void initializeGL(QGLPainter *painter);

    /// Virtual function required by QGLView.
    /// Paint the room and HUD interface.
    void paintGL(QGLPainter *painter);

    /// Handler for mouse press events.
    /// Pick up the item if pressed on a file entry,
    /// start roaming if pressed on blank place,
    /// do nothing otherwise (practical actions will take place on release).
    void mousePressEvent(QMouseEvent *event);

    /// Handler for mouse release events.
    /// Make response based on the release position and current state
    /// (is any item picked up yet and is it roaming now).
    void mouseReleaseEvent(QMouseEvent *event);

    /// Handler for mouse move events.
    /// Move the item if an item is picked up,
    /// rotate the camera if is roaming now.
    void mouseMoveEvent(QMouseEvent *event);

    /// Handler for keyboard shortcuts.
    /// The shortcuts are mainly for debug purpose,
    /// any important actions can be done by mouse.
    void keyPressEvent(QKeyEvent *event);

    /// Resize the window.
    void resizeEvent(QResizeEvent *);

    /// Do nothing for wheel events.
    void wheelEvent(QWheelEvent *);

private:
    // constructor helpers
    void setupAnimation();
    void setupLight();

    // paintGL helpers
    void updateCamera();
    void paintCurrentRoom(QGLPainter *painter);
    void paintNextRoom(QGLPainter *painter);
    void paintHud(QGLPainter *painter);

    void updateHudContent(qreal x = 0, qreal y = 0, QString text = QString());
    void paintOutline(QGLPainter *painter);

    // left-click actions
    void invokeObject(int id);
    void openEntry(int index);

    // hovering control
    void hoverEnter(int obj);
    void hoverLeave();

    // animation
    void startAnimation(AnimStage stage);
    void finishAnimation();

    // roaming
    void startRoaming(QPoint pos);

    // main members
    Directory *dir;
    Room *curRoom;

    // pseudo-const variables
    QVector3D defaultCenter;
    QVector3D defaultEye;

    // picked file entry
    int pickedEntry = -1;
    QVector3D deltaPos;
    QVector3D pickedPos;
    qreal pickedDepth;
    bool isNear = false;

    // text, outline, etc
    QGLSceneNode *hud;
    OutlinePainter *outline;

    QOpenGLFramebufferObject *fbo = NULL;
    QGLFramebufferObjectSurface *surface = NULL;

    // roaming
    bool isRoaming = false;
    QPoint roamStartPos;
    QVector3D roamStartCenter;

    // animation
    QVariantAnimation *animation;
    AnimStage animStage;
    qreal animProg;

    int enteringDir = -1;
    int leavingDoor = -1;

    QVector3D startCenter;
    QVector3D startEye;
    QVector3D startUp;

    QVector3D deltaCenter;
    QVector3D deltaEye;
    QVector3D deltaUp;

    // light
    QGLLightParameters *light;
    int lightId;

    QGLShaderProgramEffect *phongEffect;
    QGLShaderProgramEffect *boxEffect;
};

#endif
