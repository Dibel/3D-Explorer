#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>
#include <QtGui/QImage>
#include "pickobject.h"

class ImageObject;
class MeshObject;
class Directory;
class PickSurface;
class QGLFramebufferObjectSurface;
class QPaintDevice;
class QGLAbstractScene;
class QGLMaterial;
class QGLSceneNode;
class QGLShaderProgramEffect;
class QGLTexture2D;
class QVariantAnimation;

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
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *);
    void resizeEvent(QResizeEvent *);

private:
    enum { MaxBox = MaxBoxId, TrashBin, Door, LeftArrow, RightArrow, Picture = StartImageId };

    static const int roomSize = 80;
    static const int roomHeight = 120;
    static const int eyeHeight = 50;
    static const qreal boxScale;

    void loadModels();
    void setupObjects();

    void paintHud(qreal x, qreal y, QString text);
    void loadDir(const QVector<MeshObject*> &boxes, ImageObject *picture);
    void hoverEnter(MeshObject *object);
    void hoverLeave();
    void finishAnimation();

    void debugFunc();

    MeshObject *enteringDir;
    bool isLeavingDir;
    qreal animProg;
    QVariantAnimation *animation;
    QVector3D startCenter;
    QVector3D startEye;
    QVector3D startUp;
    QVector3D deltaCenter;
    QVector3D deltaEye;
    QVector3D deltaUp;
    int animationStage;

    /* temporary materials for debug purpose */
    QGLMaterial *mat1;
    QGLMaterial *mat2;

    QVector<MeshObject*> staticMeshes;
    QVector<MeshObject*> boxes;
    QVector<MeshObject*> backBoxes;
    MeshObject *floor;
    MeshObject *ceil;

    ImageObject *picture;
    ImageObject *backPicture;
    ImageObject *hudObject;
    ImageObject *outline;

    QGLSceneNode *dirModel;
    QGLSceneNode *fileModel;

    bool isRotating;
    QPoint pressPos;
    QVector3D oldCameraCenter;

    QGLLightParameters *light;
    QGLLightParameters *light2;

    Directory *dir;

    int slotCnt;

    QMatrix4x4 mvp;

    MeshObject *pickedObject;
    /* original position of picked object */
    QVector3D pickedPos;
    /* clicked position in picked object's local coordinate */
    QVector3D pickedModelPos;
    /* picked object's depth in projected coordinate */
    qreal pickedDepth;

    MeshObject *enteredObject;
    QOpenGLFramebufferObject *fbo;
    PickSurface *surface;

    QGLShaderProgramEffect *phongEffect;
    QGLShaderProgramEffect *boxEffect;
    bool isShowingFileName;
};

#endif
