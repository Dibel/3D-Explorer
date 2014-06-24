#ifndef SCENE_H
#define SCENE_H

//#include <Qt3D/QGLView>
#include "lib/glview.h"

//class Config;
class Directory;
class ImageObject;
class PickObject;
class Room;
class Surface;

enum AnimStage : int;

class QGLShaderProgramEffect;
class QVariantAnimation;


class View : public GLView {
    Q_OBJECT
public:
    View(int width = 800, int height = 600);
    ~View();

    void load();

protected:
    void initializeGL(QGLPainter *painter);
    void paintGL(QGLPainter *painter);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);

private:
    void loadDir(bool back = false);

    //Config *config;
    Directory *dir;
    Room *curRoom;

    ImageObject *picture;
    ImageObject *backPicture;

    QVector3D defaultCenter;
    QVector3D defaultEye;

    // constructor helper
    void setupAnimation();
    void setupLight();
    void setupObjects();


    // object control

    /* handle clicked object */
    void invokeObject(int id);
    /* open file or directory */
    void openEntry(int index);

    int pickedObject;
    QVector3D deltaPos;
    /* original position of picked object */
    QVector3D pickedPos;
    /* clicked position in picked object's local coordinate */
    QVector3D pickedModelPos;
    /* picked object's depth in projected coordinate */
    qreal pickedDepth;
    /* whether the movement is by accident */
    bool isNear = false;


    // hover object

    void hoverEnter(int obj);
    void hoverLeave();

    void paintHud(qreal x = 0, qreal y = 0, QString text = QString());
    void paintOutline(int obj);

    int hoveringObject;

    /* show file name and path info on HUD */
    ImageObject *hudObject;
    /* outline hovering object */
    ImageObject *outline;

    /* buffer for painting outline */
    QOpenGLFramebufferObject *fbo;
    Surface *surface;


    // roaming

    void startRoaming(QPoint pos);

    bool isRoaming;
    /* starting position of roaming */
    QPoint roamStartPos;
    /* camera center when roaming start */
    QVector3D roamStartCenter;


    // animation

    void startAnimation(AnimStage stage);
    void finishAnimation();

    QVariantAnimation *animation;
    AnimStage animStage;
    qreal animProg;

    int enteringDir;
    int leavingDoor;

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


    // misc

    void debugFunc();
    QMatrix4x4 mvp;
    bool isShowingFileName;
};

#endif
