#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>
#include <QtGui/QImage>
#include <QtCore/QHash>
#include "pickobject.h"

#include "room.h"

class ImageObject;
class MeshObject;
class Directory;
class Surface;
class QGLFramebufferObjectSurface;
class QPaintDevice;
class QGLAbstractScene;
class QGLMaterial;
class QGLMaterialCollection;
class QGLSceneNode;
class QGLShaderProgramEffect;
class QGLTexture2D;
class QVariantAnimation;
enum AnimStage : int;

class View : public QGLView {
    Q_OBJECT
public:
    View(int width = 800, int height = 600);
    ~View();

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

    static const QVector3D defaultCenter;
    static const QVector3D defaultEye;


    void loadModels();
    void setupObjects();
    void loadDir(const QVector<MeshObject*> &boxes, ImageObject *picture);
    void hoverEnter(MeshObject *object);
    void hoverLeave();
    void startAnimation(AnimStage stage);
    void finishAnimation();
    void debugFunc();

    void paintHud(qreal x = 0, qreal y = 0, QString text = QString());
    void paintOutline(MeshObject *obj);

    void startRotate(QPoint pos);
    void invokeObject(PickObject *obj);
    void openEntry(MeshObject *obj);

    Room *curRoom;

    MeshObject *enteringDir;
    MeshObject *leavingDoor;
    qreal animProg;
    QVariantAnimation *animation;
    AnimStage animStage;

    QVector3D startCenter;
    QVector3D startEye;
    QVector3D startUp;
    QVector3D deltaCenter;
    QVector3D deltaEye;
    QVector3D deltaUp;

    /* temporary materials for debug purpose */
    QGLMaterial *mat1;
    QGLMaterial *mat2;

    /* FIXME: unable to use QGLMaterialCollection */
    //QSharedPointer<QGLMaterialCollection> palette;
    QHash<QString, QGLMaterial*> palette;

    ImageObject *picture;
    ImageObject *backPicture;
    ImageObject *hudObject;
    ImageObject *outline;

    bool isRotating;
    QPoint pressPos;
    QVector3D oldCameraCenter;

    QGLLightParameters *light;
    int lightId;

    Directory *dir;

    QMatrix4x4 mvp;

    MeshObject *pickedObject;
    /* original position of picked object */
    QVector3D pickedPos;
    /* clicked position in picked object's local coordinate */
    QVector3D pickedModelPos;
    /* picked object's depth in projected coordinate */
    qreal pickedDepth;
    bool isNear;

    MeshObject *enteredObject;
    QOpenGLFramebufferObject *fbo;
    Surface *surface;

    QGLShaderProgramEffect *phongEffect;
    QGLShaderProgramEffect *boxEffect;
    bool isShowingFileName;
};

#endif
