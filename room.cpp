#include "room.h"
#include "meshobject.h"
#include "common.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLMaterial>
#include <Qt3D/QGLView>
#include <QtCore/QFile>

static int id[4] = { -1, -1, TrashBin, Door };

static void FixNodesRecursive(int matIndex, QGLSceneNode* pNode)
{
    pNode->setMaterialIndex(matIndex);
    pNode->setEffect(QGL::LitModulateTexture2D);
    for (auto node : pNode->allChildren()) {
        node->setMaterialIndex(matIndex);
        node->setEffect(QGL::LitModulateTexture2D);
    };
}

void Room::paintCurRoom(QGLPainter *painter, MeshObject *animObj, qreal animProg) {
    for (MeshObject *obj : solid) {
        if (obj == animObj) {
            qreal old = obj->rotationAngle();
            obj->setRotationAngle(old - 90.0 * animProg);
            obj->draw(painter);
            obj->setRotationAngle(old);
        } else
            obj->draw(painter);
    }

    floor->draw(painter);
    ceil->draw(painter);

    for (MeshObject *obj : entry)
        if (obj->pickType() != MeshObject::Picked)
            obj->draw(painter);
}

void Room::paintNextRoom(QGLPainter *painter, int stage) {
    for (MeshObject *obj : solid)
        obj->draw(painter);

    if (stage == 0) {
        floor->setPosition(floor->position() + QVector3D(0, 0.001, 0));
        floor->draw(painter);
        floor->setPosition(floor->position() - QVector3D(0, 0.001, 0));
    } else {
        floor->draw(painter);
        ceil->draw(painter);
    }

    if (stage != 1)
        for (MeshObject *obj : backEntry)
            obj->draw(painter);
}


Room::Room(const QString &fileName, const QHash<QString, QGLMaterial*> &palette, QGLView *view) : slotNum(0), palette(palette), view(view) {
    loadDefaultModels();

    QFile file(":/config/" + fileName);
    file.open(QIODevice::ReadOnly);
    QString line, property;
    QTextStream stream;
    stream.setString(&line, QIODevice::ReadOnly);

    while (!file.atEnd()) {
        line = file.readLine();
        if (line.isEmpty() || line[0] == '#' || line[0] == '\n') continue;
        if (line[0] == '[') {
            property = line.mid(1, line.indexOf(']') - 1);
        } else {
            stream.seek(0);
            loadProperty(property, stream);
        }
    }

    file.close();
}

void Room::loadProperty(const QString &property, QTextStream &value) {
    if (property == "model") {
        loadModel(value);

    } else if (property == "cdup") {
        qreal x, y, z, angle;
        value >> x >> y >> z >> angle;
        cdUpPosition = QVector3D(x, y, z);
        cdUpDirection = angle;

    } else if (property == "wall") {
        loadWall(value);

    } else if (property == "material") {
        QString floorMat, ceilMat;
        value >> floorMat >> ceilMat;
        floor->model()->setMaterial(palette[floorMat]);
        ceil->model()->setMaterial(palette[ceilMat]);

    }
}

void Room::loadModel(QTextStream &value) {
    QString name, mat, extra;
    qreal x, y, z, w, angle;
    int type, recursive, tmp;
    value >> name >> type >> x >> y >> z >> w >> angle >> mat >> recursive;

    QGLAbstractScene *model = QGLAbstractScene::loadScene(":/model/" + name + ".obj");
    model->setParent(view);

    if (recursive) {
        int matIndex = model->mainNode()->palette()->addMaterial(palette[mat]);
        FixNodesRecursive(matIndex, model->mainNode());
    } else
        model->mainNode()->setMaterial(palette[mat]);

    MeshObject *mesh = new MeshObject(model, view, id[type]);
    mesh->setPosition(QVector3D(x, y, z));
    mesh->setScale(w);
    mesh->setRotationVector(QVector3D(0, 1, 0));
    mesh->setRotationAngle(angle);

    QFile file2;
    QTextStream stream2;

    switch (type) {
        case 1:
            file2.setFileName(":/config/" + name + ".conf");
            file2.open(QFile::ReadOnly);
            stream2.setDevice(&file2);

            stream2 >> tmp >> mat;
            slotNum += tmp;
            for (int i = 0; i < tmp; ++i) {
                stream2 >> x >> y >> z;
                MeshObject *box = new MeshObject(dirModel, view, entry.size());
                box->setMaterial(palette[mat]);
                box->setPosition(QVector3D(x, y, z) + mesh->position());
                entry.push_back(box);

                box = new MeshObject(dirModel, view, -2);
                box->setMaterial(palette[mat]);
                box->setPosition(QVector3D(x, y, z) + mesh->position());
                backEntry.push_back(box);
            }

            stream2.setDevice(NULL);
            file2.close();
            break;

        case 3:
            value >> extra;
            tmp = extra.toInt();
            model->mainNode()->setX(-tmp);
            mesh->setPosition(QVector3D(x, y, z) + rotateCcw(tmp, 0, 0, angle));
            mesh->setInfo(extra);
            break;

        default:
            qDebug() << "no config";
    }

    solid << mesh;
}

void Room::loadWall(QTextStream &value) {
    int side;
    qreal l, r, h;
    QString mat;
    value >> side >> l >> r >> h >> mat;

    MeshObject *mesh;
    if (l == -1) {
        QGLBuilder roomBuilder;
        roomBuilder.newSection(QGL::Faceted);
        roomBuilder.addPane(QSizeF(roomSize * 2, roomHeight));
        QGLSceneNode *pane = roomBuilder.finalizedSceneNode();
        pane->setMaterial(palette[mat]);
        pane->setPosition(QVector3D(0, roomHeight * 0.5, 0));

        mesh = new MeshObject(pane, view, -1);
    } else {
        /* room, a cube that inside-out */
        QVector3DArray wallVertices;
        wallVertices.append(-roomSize, 0, 0);
        wallVertices.append(l, 0, 0);
        wallVertices.append(-roomSize, roomHeight, 0);
        wallVertices.append(l, h, 0);
        wallVertices.append(roomSize, roomHeight, 0);
        wallVertices.append(r, h, 0);
        wallVertices.append(roomSize, 0, 0);
        wallVertices.append(r, 0, 0);

        QGeometryData wallStrip;
        wallStrip.appendVertexArray(wallVertices);
        QGLBuilder wallBuilder;
        wallBuilder.newSection(QGL::Faceted);
        wallBuilder.addTriangleStrip(wallStrip);
        QGLSceneNode *wall = wallBuilder.finalizedSceneNode();
        wall->setMaterial(palette[mat]);

        mesh = new MeshObject(wall, view, -1);
    }

    mesh->setPosition(rotateCcw(0, 0, -roomSize, side * 90));
    mesh->setRotationVector(QVector3D(0, 1, 0));
    mesh->setRotationAngle(side * 90);

    solid << mesh;
}

void Room::loadDefaultModels() {
    QMatrix4x4 dirTrans;
    dirTrans.scale(QVector3D(roomSize * boxScale * 2, roomHeight * boxScale, roomSize * boxScale * 2));
    dirTrans.translate(QVector3D(0, 0.5, 0));
    QGLBuilder dirBuilder;
    dirBuilder.newSection(QGL::Faceted);
    dirBuilder << QGLCube(1);
    dirBuilder.currentNode()->setLocalTransform(dirTrans);
    dirModel = dirBuilder.finalizedSceneNode();
    dirModel->setParent(view);

    QMatrix4x4 trans;
    trans.scale(QVector3D(0.5, 1, 1));
    QGLBuilder fileBuilder;
    fileBuilder.newSection(QGL::Faceted);
    fileBuilder << QGLCube(6);
    fileBuilder.currentNode()->setY(3);
    fileBuilder.currentNode()->setLocalTransform(trans);
    fileModel = fileBuilder.finalizedSceneNode();
    fileModel->setParent(view);

    QMatrix4x4 floorTrans;
    floorTrans.rotate(90, -1, 0, 0);
    QGLBuilder floorBuilder;
    floorBuilder.newSection(QGL::Faceted);
    floorBuilder.addPane(roomSize * 2);
    floorBuilder.currentNode()->setLocalTransform(floorTrans);
    QGLSceneNode *floorNode = floorBuilder.finalizedSceneNode();

    floorNode->setEffect(QGL::LitModulateTexture2D);
    floor = new MeshObject(floorNode, view, -1);

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(roomSize * 2);
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();

    ceil = new MeshObject(ceilNode, view, -1);
}
