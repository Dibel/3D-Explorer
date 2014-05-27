#include "room.h"
#include "meshobject.h"
#include "common.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLMaterial>
#include <Qt3D/QGLView>
#include <QtCore/QFile>

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
    Room *room = this;
    for (MeshObject *obj : room->solid) {
        if (obj == animObj) {
            qreal old = obj->rotationAngle();
            obj->setRotationAngle(old - 90.0 * animProg);
            obj->draw(painter);
            obj->setRotationAngle(old);
        } else
            obj->draw(painter);
    }

    room->floor->draw(painter);
    room->ceil->draw(painter);

    for (MeshObject *obj : room->entry)
        if (obj->pickType() != MeshObject::Picked)
            obj->draw(painter);
}

void Room::paintNextRoom(QGLPainter *painter, int stage) {
    Room *room = this;
    for (MeshObject *obj : room->solid)
        obj->draw(painter);

    if (stage == 0) {
        room->floor->setPosition(room->floor->position() + QVector3D(0, 0.001, 0));
        room->floor->draw(painter);
        room->floor->setPosition(room->floor->position() - QVector3D(0, 0.001, 0));
    } else {
        room->floor->draw(painter);
        room->ceil->draw(painter);
    }

    if (stage != 1)
        for (MeshObject *obj : room->backEntry)
            obj->draw(painter);
}


Room::Room(const QString &fileName, const QHash<QString, QGLMaterial*> &palette, QGLView *view) {
    Room *room = this;

    QMatrix4x4 dirTrans;
    dirTrans.scale(QVector3D(roomSize * boxScale * 2, roomHeight * boxScale, roomSize * boxScale * 2));
    dirTrans.translate(QVector3D(0, 0.5, 0));
    QGLBuilder dirBuilder;
    dirBuilder.newSection(QGL::Faceted);
    dirBuilder << QGLCube(1);
    dirBuilder.currentNode()->setLocalTransform(dirTrans);
    room->dirModel = dirBuilder.finalizedSceneNode();
    room->dirModel->setParent(view);

    QMatrix4x4 trans;
    trans.scale(QVector3D(0.5, 1, 1));
    QGLBuilder fileBuilder;
    fileBuilder.newSection(QGL::Faceted);
    fileBuilder << QGLCube(6);
    fileBuilder.currentNode()->setY(3);
    fileBuilder.currentNode()->setLocalTransform(trans);
    room->fileModel = fileBuilder.finalizedSceneNode();
    room->fileModel->setParent(view);

    static int id[4] = { -1, -1, TrashBin, Door };
    QString name, extra;
    QGLAbstractScene *model;
    int matIndex;
    MeshObject *mesh;
    QFile file2;
    QTextStream stream2;

    room->slotNum = 0;

    QFile file(":/config/" + fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream stream;

    QString line;

    stream.setString(&line, QIODevice::ReadOnly);

    while (line != "[model]\n") line = file.readLine();

    while (true) {
        line = file.readLine();
        if (line.isEmpty() || line[0] == '[') break;
        if (line[0] == '#' || line[0] == '\n') continue;

        stream.seek(0);

        QString mat;
        qreal x, y, z, w, angle;
        int type, recursive, tmp;
        stream >> name >> type >> x >> y >> z >> w >> angle >> mat >> recursive >> extra;

        model = QGLAbstractScene::loadScene(":/model/" + name + ".obj");
        model->setParent(view);

        if (recursive) {
            matIndex = model->mainNode()->palette()->addMaterial(palette[mat]);
            FixNodesRecursive(matIndex, model->mainNode());
        } else
            model->mainNode()->setMaterial(palette[mat]);

        mesh = new MeshObject(model, view, id[type]);
        mesh->setPosition(QVector3D(x, y, z));
        mesh->setScale(w);
        mesh->setRotationVector(QVector3D(0, 1, 0));
        mesh->setRotationAngle(angle);

        switch (type) {
            case 1:
                file2.setFileName(":/config/" + extra);
                file2.open(QFile::ReadOnly);
                stream2.setDevice(&file2);

                stream2 >> tmp >> mat;
                room->slotNum += tmp;
                for (int i = 0; i < tmp; ++i) {
                    stream2 >> x >> y >> z;
                    MeshObject *box = new MeshObject(room->dirModel, view, room->entry.size());
                    box->setMaterial(palette[mat]);
                    box->setPosition(QVector3D(x, y, z) + mesh->position());
                    room->entry.push_back(box);

                    box = new MeshObject(room->dirModel, view, -2);
                    box->setMaterial(palette[mat]);
                    box->setPosition(QVector3D(x, y, z) + mesh->position());
                    room->backEntry.push_back(box);
                }

                stream2.setDevice(NULL);
                file2.close();
                break;

            case 3:
                tmp = extra.toInt();
                model->mainNode()->setX(-tmp);
                mesh->setPosition(QVector3D(x, y, z) + rotateCcw(tmp, 0, 0, angle));
                mesh->setInfo(extra);
                break;

            default:
                qDebug() << "no config";
        }

        room->solid << mesh;
    }

    while (line != "[cdup]\n") line = file.readLine();

    while (true) {
        line = file.readLine();
        if (line.isEmpty() || line[0] == '[') break;
        if (line[0] == '#' || line[0] == '\n') continue;

        stream.seek(0);

        qreal x, y, z, angle;
        stream >> x >> y >> z >> angle;

        room->cdUpPosition = QVector3D(x, y, z);
        room->cdUpDirection = angle;
    }

    while (line != "[wall]\n") line = file.readLine();

    QGLBuilder roomBuilder;
    roomBuilder.newSection(QGL::Faceted);
    roomBuilder.addPane(QSizeF(roomSize * 2, roomHeight));
    QGLSceneNode *pane = roomBuilder.finalizedSceneNode();
    pane->setMaterial(palette["room"]);
    pane->setPosition(QVector3D(0, roomHeight * 0.5, 0));

    while (true) {
        line = file.readLine();
        if (line.isEmpty() || line[0] == '[') break;
        if (line[0] == '#' || line[0] == '\n') continue;

        stream.seek(0);

        for (int i = 0; i < 4; ++i) {
            qreal l, r, h;
            stream >> l >> r >> h;

            if (l == -1) {
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
                wall->setMaterial(palette["wall"]);

                mesh = new MeshObject(wall, view, -1);
            }

            mesh->setPosition(rotateCcw(0, 0, -roomSize, 90 * i));
            mesh->setRotationVector(QVector3D(0, 1, 0));
            mesh->setRotationAngle(90 * i);

            room->solid << mesh;
        }
    }

    QMatrix4x4 floorTrans;
    floorTrans.rotate(90, -1, 0, 0);
    QGLBuilder floorBuilder;
    floorBuilder.newSection(QGL::Faceted);
    floorBuilder.addPane(roomSize * 2);
    floorBuilder.currentNode()->setLocalTransform(floorTrans);
    QGLSceneNode *floorNode = floorBuilder.finalizedSceneNode();

    floorNode->setMaterial(palette["wood"]);
    floorNode->setEffect(QGL::LitModulateTexture2D);
    room->floor = new MeshObject(floorNode, view, -1);

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(roomSize * 2);
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();

    ceilNode->setMaterial(palette["ceil"]);
    room->ceil = new MeshObject(ceilNode, view, -1);

    file.close();
}
