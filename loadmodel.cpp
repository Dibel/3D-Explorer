#include "view.h"
#include "meshobject.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLMaterialCollection>
#include <QSharedPointer>

static void FixNodesRecursive(int matIndex, QGLSceneNode* pNode)
{
    pNode->setMaterialIndex(matIndex);
    pNode->setEffect(QGL::LitModulateTexture2D);
    for (auto node : pNode->allChildren()) {
        node->setMaterialIndex(matIndex);
        node->setEffect(QGL::LitModulateTexture2D);
    };
}

void View::loadModels() {
    QGLMaterial *ceilMaterial = new QGLMaterial();
    ceilMaterial->setAmbientColor(QColor(255, 255, 255));
    ceilMaterial->setDiffuseColor(QColor(255, 255, 255));
    ceilMaterial->setSpecularColor(QColor(255, 255, 255));
    ceilMaterial->setShininess(120);

    QGLAbstractScene *model;
    MeshObject *mesh;
    int matIndex;

    QFile confFile("./config/main.conf");
    qDebug() << confFile.open(QIODevice::ReadOnly);
    QTextStream stream(&confFile);
    QString name = stream.readLine();

    static int id[4] = { -1, -1, TrashBin, Door };

    QMatrix4x4 doorTrans;
    doorTrans.translate(QVector3D(21, 0, 0));

    QGLTexture2D *tex;

    while (name != "[material]" && !stream.atEnd()) name = stream.readLine();
    while (true) {
        stream >> name;
        if (name[0] == '[') { stream.readLine(); break; }
        if (name[0] == '#') { stream.readLine(); continue; }

        QGLMaterial *m = new QGLMaterial();

        if (name != "-") {
            tex = new QGLTexture2D();
            tex->setImage(QImage(":/maps/" + name));
            m->setTexture(tex);
        }

        int r, g, b;
        stream >> r >> g >> b;
        if (r != -1) m->setAmbientColor(QColor(r, g, b));
        stream >> r >> g >> b;
        if (r != -1) m->setDiffuseColor(QColor(r, g, b));
        stream >> r >> g >> b;
        if (r != -1) m->setSpecularColor(QColor(r, g, b));
        stream >> r; m->setShininess(r);

        if (name == "floor.jpg") m->setTextureCombineMode(QGLMaterial::Decal);

        palette.push_back(m);
        stream.readLine();
    }

    mat2 = palette[2];

    while (name != "[model]" && !stream.atEnd()) name = stream.readLine();
    while (true) {
        stream >> name;
        if (name[0] == '[') { stream.readLine(); break; }
        if (name[0] == '#') { stream.readLine(); continue; }

        int type, matId, recursive;
        qreal x, y, z, w, angle;
        stream >> type >> x >> y >> z >> w >> angle >> matId >> recursive;

        model = QGLAbstractScene::loadScene(":/model/" + name + ".obj");
        model->setParent(this);

        if (recursive) {
            matIndex = model->mainNode()->palette()->addMaterial(palette[0]);
            FixNodesRecursive(matIndex, model->mainNode());
        } else
            model->mainNode()->setMaterial(palette[matId]);

        mesh = new MeshObject(model, this, id[type]);
        mesh->setPosition(QVector3D(x, y, z));
        mesh->setScale(w);

        if (type == 3) { // door
            model->mainNode()->setLocalTransform(doorTrans);
            mesh->setRotationVector(QVector3D(0, -1, 0));
        }

        staticMeshes << mesh;
        stream.readLine();
    }

    confFile.close();

    /* arrows */
    model = QGLAbstractScene::loadScene(":/model/leftarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(palette[2]);
    mesh = new MeshObject(model, this, LeftArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(-50, 90, -roomSize));
    staticMeshes << mesh;

    model = QGLAbstractScene::loadScene(":/model/rightarrow.obj");
    model->setParent(this);
    model->mainNode()->setMaterial(palette[2]);
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

    QGLMaterial *roomMaterial = new QGLMaterial();
    roomMaterial->setAmbientColor(QColor(95, 75, 58));
    roomMaterial->setDiffuseColor(QColor(143, 122, 90));
    roomMaterial->setSpecularColor(QColor(154, 135, 105));
    roomMaterial->setShininess(10);

    /* FIXME: the front wall can't share material with other walls */
    QGLMaterial *wallMaterial = new QGLMaterial();
    wallMaterial->setAmbientColor(QColor(95, 75, 58));
    wallMaterial->setDiffuseColor(QColor(143, 122, 90));
    wallMaterial->setSpecularColor(QColor(154, 135, 105));
    wallMaterial->setShininess(10);

    /* room, a cube that inside-out */
    QGLBuilder roomBuilder;
    roomBuilder.newSection(QGL::Faceted);
    roomBuilder.addPane(QSizeF(roomSize * 2, roomHeight));
    QGLSceneNode *pane = roomBuilder.finalizedSceneNode();
    pane->setMaterial(roomMaterial);
    pane->setPosition(QVector3D(0, roomHeight * 0.5, 0));

    QVector3DArray wallVertices;
    wallVertices.append(-roomSize, 0, 0);
    wallVertices.append(4, 0, 0);
    wallVertices.append(-roomSize, roomHeight, 0);
    wallVertices.append(4, 95, 0);
    wallVertices.append(roomSize, roomHeight, 0);
    wallVertices.append(54, 95, 0);
    wallVertices.append(roomSize, 0, 0);
    wallVertices.append(54, 0, 0);

    QGeometryData wallStrip;
    wallStrip.appendVertexArray(wallVertices);
    QGLBuilder wallBuilder;
    wallBuilder.newSection(QGL::Faceted);
    wallBuilder.addTriangleStrip(wallStrip);
    QGLSceneNode *wall = wallBuilder.finalizedSceneNode();
    wall->setMaterial(wallMaterial);

    MeshObject *front = new MeshObject(pane, this, -1);
    front->setPosition(QVector3D(0, 0, -roomSize));

    MeshObject *right = new MeshObject(pane, this, -1);
    right->setPosition(QVector3D(-roomSize, 0, 0));
    right->setRotationVector(QVector3D(0, 1, 0));
    right->setRotationAngle(90);

    MeshObject *back = new MeshObject(wall, this, -1);
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

    floorNode->setMaterialIndex(matIndex);
    floorNode->setEffect(QGL::LitModulateTexture2D);
    floor = new MeshObject(floorNode, this, -1);

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(roomSize * 2);
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();


    ceilNode->setMaterial(ceilMaterial);
    ceil = new MeshObject(ceilNode, this, -1);
}
