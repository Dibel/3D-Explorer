#ifndef SCENE_H
#define SCENE_H

#include <Qt3D/QGLView>
#include <QtGui/QImage>
#include <QtCore/QDir>

class ImageObject;
class MeshObject;
class QPaintDevice;
class QGLAbstractScene;
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
    void initializeBox();
    QImage paintHud(float x, float y, QString text);
    void nextPicture();
    void updateDir(const QVector<MeshObject*> &boxes, ImageObject *picture);
    void hoverEnter(MeshObject *object);
    void hoverLeave();
    void finishAnimation();

    MeshObject *enteringDir;
    qreal animProg;
    QVariantAnimation *animation;

    QVector<MeshObject*> background;
    QVector<MeshObject*> boxes;
    QVector<MeshObject*> backBoxes;
    MeshObject *trashBin;
    ImageObject *picture;
    ImageObject *backPicture;
    ImageObject *hudObject;

    QVector<QGLAbstractScene*> backgroundModels;
    QGLAbstractScene *trashBinModel;
    QGLSceneNode *dirModel;
    QGLSceneNode *fileModel;

    QDir dir;
    int entryCnt;
    int dirEntryCnt;
    int slotCnt;
    int pageCnt;
    QStringList pictureList;
    int currentPicture;

    QMatrix4x4 mvp;

    MeshObject *pickedObject;
    /* original position of picked object */
    QVector3D pickedPos;
    /* clicked position in picked object's local coordinate */
    QVector3D pickedModelPos;
    /* picked object's depth in projected coordinate */
    qreal pickedDepth;

    MeshObject *enteredObject;
};

#endif
