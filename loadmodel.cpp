#include "view.h"
#include "meshobject.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>

void View::loadModels() {
    mat1 = new QGLMaterial(this);
    mat1->setAmbientColor(QColor(192, 150, 128));
    mat1->setSpecularColor(QColor(60, 60, 60));
    mat1->setShininess(20);

    mat2 = new QGLMaterial(this);
    mat2->setAmbientColor(QColor(255, 255, 255));
    mat2->setDiffuseColor(QColor(150, 150, 150));
    mat2->setSpecularColor(QColor(255, 255, 255));
    mat2->setShininess(20);

    QGLAbstractScene *model;
    MeshObject *mesh;

    /* shelf */
    model = QGLAbstractScene::loadScene(":/model/shelf.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    model->mainNode()->setPosition(QVector3D(0, 0, -roomSize));
    staticMeshes << new MeshObject(model, this);

    /* photo frame */
    model = QGLAbstractScene::loadScene(":/model/frame.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    model->mainNode()->setPosition(QVector3D(-50, 50, -roomSize));
    staticMeshes << new MeshObject(model, this);

    /* trash bin */
    model = QGLAbstractScene::loadScene(":/model/trash.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    QMatrix4x4 trashBinTrans;
    trashBinTrans.translate(QVector3D(-40, 0, 10 - roomSize));
    trashBinTrans.scale(0.2);
    model->mainNode()->setLocalTransform(trashBinTrans);
    staticMeshes << new MeshObject(model, this, TrashBin);

    /* door */
    model = QGLAbstractScene::loadScene(":/model/door.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat1);
    QMatrix4x4 doorTrans;
    doorTrans.translate(QVector3D(50, 36, -roomSize));
    doorTrans.scale(10);
    model->mainNode()->setLocalTransform(doorTrans);
    staticMeshes << new MeshObject(model, this, Door);

    /* arrows */
    model = QGLAbstractScene::loadScene(":/model/leftarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat2);
    mesh = new MeshObject(model, this, LeftArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(-50, 90, -roomSize));
    staticMeshes << mesh;

    model = QGLAbstractScene::loadScene(":/model/rightarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(mat2);
    mesh = new MeshObject(model, this, RightArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(50, 90, -roomSize));
    staticMeshes << mesh;

    /* directory and file boxes */
    QMatrix4x4 dirTrans;
    dirTrans.scale(QVector3D(roomSize * boxScale * 2, roomHeight * boxScale, roomSize * boxScale * 2));
    dirTrans.translate(QVector3D(0, 0.5, 0));
    QGLBuilder dirBuilder;
    dirBuilder.newSection(QGL::Faceted);
    dirBuilder << QGLCube(1);
    dirBuilder.currentNode()->setLocalTransform(dirTrans);
    //dirBuilder.currentNode()->setY(3);
    dirModel = dirBuilder.finalizedSceneNode();
    dirModel->setParent(this);

    QMatrix4x4 trans;
    trans.scale(QVector3D(0.5, 1, 1));
    QGLBuilder fileBuilder;
    fileBuilder.newSection(QGL::Faceted);
    fileBuilder << QGLCube(6);
    fileBuilder.currentNode()->setY(3);
    fileBuilder.currentNode()->setLocalTransform(trans);
    fileModel = fileBuilder.finalizedSceneNode();
    fileModel->setParent(this);

    /* room, a cube that inside-out */
    QGLBuilder roomBuilder;
    roomBuilder.newSection(QGL::Faceted);
    roomBuilder.addPane(QSizeF(roomSize * 2, roomHeight));
    QGLSceneNode *pane = roomBuilder.finalizedSceneNode();
    QGLMaterial *roomMaterial = new QGLMaterial();
    roomMaterial->setAmbientColor(QColor(95, 75, 58));
    roomMaterial->setDiffuseColor(QColor(143, 122, 90));
    roomMaterial->setSpecularColor(QColor(154, 135, 105));
    roomMaterial->setShininess(10);
    pane->setMaterial(roomMaterial);
    //pane->setBackMaterial(mat2);
    pane->setPosition(QVector3D(0, roomHeight * 0.5, 0));

    MeshObject *front = new MeshObject(pane, this, -1);
    front->setPosition(QVector3D(0, 0, -roomSize));

    MeshObject *right = new MeshObject(pane, this, -1);
    right->setPosition(QVector3D(-roomSize, 0, 0));
    right->setRotationVector(QVector3D(0, 1, 0));
    right->setRotationAngle(90);

    MeshObject *back = new MeshObject(pane, this, -1);
    back->setPosition(QVector3D(0, 0, roomSize));
    back->setRotationVector(QVector3D(0, 1, 0));
    back->setRotationAngle(180);

    MeshObject *left = new MeshObject(pane, this, -1);
    left->setPosition(QVector3D(roomSize, 0, 0));
    left->setRotationVector(QVector3D(0, 1, 0));
    left->setRotationAngle(270);

    staticMeshes << front << right << back << left;

    QMatrix4x4 floorTrans;
    floorTrans.rotate(90, -1, 0, 0);
    QGLBuilder floorBuilder;
    floorBuilder.newSection(QGL::Faceted);
    floorBuilder.addPane(roomSize * 2);
    floorBuilder.currentNode()->setLocalTransform(floorTrans);
    QGLSceneNode *floorNode = floorBuilder.finalizedSceneNode();

    QGLTexture2D *floorTexture = new QGLTexture2D();
    QImage newImage;
    newImage.load(":/maps/floor.jpg");
    floorTexture->setImage(newImage);
    QGLMaterial *floorMaterial = new QGLMaterial();
    floorMaterial->setTexture(floorTexture);
    floorMaterial->setDiffuseColor(QColor(255, 255, 255));
    floorMaterial->setSpecularColor(QColor(255, 255, 255));
    floorMaterial->setShininess(60);
    floorMaterial->setTextureCombineMode(QGLMaterial::Decal);
    floorNode->setMaterial(floorMaterial);
    floorNode->setEffect(QGL::LitDecalTexture2D);
    floor = new MeshObject(floorNode, this, -1);

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(roomSize * 2);
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();

    QGLMaterial *ceilMaterial = new QGLMaterial();
    ceilMaterial->setAmbientColor(QColor(255, 255, 255));
    ceilMaterial->setDiffuseColor(QColor(255, 255, 255));
    ceilMaterial->setSpecularColor(QColor(255, 255, 255));
    ceilMaterial->setShininess(40);
    ceilNode->setMaterial(ceilMaterial);
    ceil = new MeshObject(ceilNode, this, -1);
}
