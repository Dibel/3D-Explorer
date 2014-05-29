#include "room.h"
#include "meshobject.h"
#include "common.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLMaterial>
#include <Qt3D/QGLView>
#include <QtCore/QFile>

static void setAllMaterial(QGLSceneNode *node, QGLMaterial *mat)
{
    int index = node->palette()->addMaterial(mat);
    node->setMaterialIndex(index);
    node->setEffect(QGL::LitModulateTexture2D);
    for (QGLSceneNode *child : node->allChildren()) {
        child->setMaterialIndex(index);
        child->setEffect(QGL::LitModulateTexture2D);
    }
}

void Room::paintCurRoom(QGLPainter *painter, MeshObject *animObj, qreal animProg) {
    for (MeshObject *obj : solid) {
        if (obj == animObj) {
            obj->setAnimAngle(90.0 * animProg);
            obj->draw(painter);
            obj->setAnimAngle(0);
        } else
            obj->draw(painter);
    }

    floor->draw(painter);
    ceil->draw(painter);

    for (MeshObject *obj : entry)
        if (obj->pickType() != MeshObject::Picked) {
            if (obj == animObj) {
                obj->setAnimAngle(90.0 * animProg);
                obj->draw(painter);
                obj->setAnimAngle(0);
            } else
                obj->draw(painter);
        }
}

void Room::paintNextRoom(QGLPainter *painter, int stage) {
    for (MeshObject *obj : solid)
        obj->draw(painter);

    if (stage == Entering1 || stage == Entering2) {
        floor->setPosition(floor->position() + QVector3D(0, 0.001, 0));
        floor->draw(painter);
        floor->setPosition(floor->position() - QVector3D(0, 0.001, 0));
    } else {
        floor->draw(painter);
        ceil->draw(painter);
    }
    
    if (stage != Leaving1 && stage != Leaving2)
        for (MeshObject *obj : backEntry)
            obj->draw(painter);
}


Room::Room(const QString &fileName, const QHash<QString, QGLMaterial*> &palette, QGLView *view)
    : slotNum(0), palette(palette), view(view)
{
    loadDefaultModels();

    QFile file(configDir + fileName);
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
        floor->mesh()->setMaterial(palette[floorMat]);
        ceil->mesh()->setMaterial(palette[ceilMat]);

    }
}

void Room::loadModel(QTextStream &value) {
    QString name, mat, extra;
    qreal x, y, z, w, angle, tmp;
    int type, recursive;
    value >> name >> type >> x >> y >> z >> w >> angle >> mat >> recursive;

    QGLAbstractScene *model = QGLAbstractScene::loadScene(dataDir + name + ".obj");
    model->setParent(view);

    if (recursive)
        setAllMaterial(model->mainNode(), palette[mat]);
    else
        model->mainNode()->setMaterial(palette[mat]);

    MeshObject *mesh = new MeshObject(model->mainNode(), view, id[type]);
    mesh->setPosition(QVector3D(x, y, z));
    mesh->setScale(w);
    mesh->setRotationVector(QVector3D(0, 1, 0));
    mesh->setRotationAngle(angle);

    QFile file2;
    QTextStream stream2;

    switch (type) {
        case 1:
            loadContainer(name, mesh);
            break;

        case 3:
            value >> tmp;
            model = QGLAbstractScene::loadScene(dataDir + name + "_anim.obj");
            model->setParent(view);
            setAllMaterial(model->mainNode(), palette[mat]);
            mesh->setAnimVector(0, -1, 0);
            mesh->setAnimCenter(tmp, 0, 0);
            mesh->setAnimMesh(model->mainNode());
            break;

        default:
            qDebug() << "no config";
    }

    solid << mesh;
}

void Room::loadContainer(const QString &name, MeshObject *mesh) {
    QFile file(configDir + name + ".conf");
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);

    int n;
    QString modelName;
    stream >> n >> modelName;
    slotNum += n;

    QGLAbstractScene *scene = QGLAbstractScene::loadScene(dataDir + modelName + ".obj");
    scene->setParent(view);
    dirSolidModel = scene->mainNode();

    scene = QGLAbstractScene::loadScene(dataDir + modelName + "_anim.obj");
    scene->setParent(view);
    dirAnimModel = scene->mainNode();

    for (int i = 0; i < n; ++i) {
        qreal x, y, z;
        stream >> x >> y >> z;
        MeshObject *obj = new MeshObject(dirSolidModel, view, entry.size());
        obj->setAnimVector(-1, 0, 0);
        obj->setAnimCenter(0, 3, -2.2);
        obj->setPosition(QVector3D(x, y, z) + mesh->position());
        entry.push_back(obj);

        obj = new MeshObject(dirSolidModel, view, -2);
        obj->setPosition(QVector3D(x, y, z) + mesh->position());
        backEntry.push_back(obj);
    }

    file.close();
}

void Room::loadWall(QTextStream &value) {
    int side;
    qreal l, r, h;
    QString mat;
    value >> side >> l >> r >> h >> mat;

    MeshObject *mesh;
    qreal w = side & 1 ? roomLength : roomWidth;
    qDebug() << side << w;
    if (l == -1) {
        QGLBuilder roomBuilder;
        roomBuilder.newSection(QGL::Faceted);
        roomBuilder.addPane(QSizeF(w, roomHeight));
        QGLSceneNode *pane = roomBuilder.finalizedSceneNode();
        pane->setMaterial(palette[mat]);
        pane->setPosition(QVector3D(0, roomHeight * 0.5, 0));

        mesh = new MeshObject(pane, view, -1);
    } else {
        /* room, a cube that inside-out */
        QVector3DArray vertices;
        vertices.append(-w / 2, 0, 0);
        vertices.append(l, 0, 0);
        vertices.append(-w / 2, roomHeight, 0);
        vertices.append(l, h, 0);
        vertices.append(w / 2, roomHeight, 0);
        vertices.append(r, h, 0);
        vertices.append(w / 2, 0, 0);
        vertices.append(r, 0, 0);

        QGeometryData strip;
        strip.appendVertexArray(vertices);
        QGLBuilder builder;
        builder.newSection(QGL::Faceted);
        builder.addTriangleStrip(strip);
        QGLSceneNode *wall = builder.finalizedSceneNode();
        wall->setMaterial(palette[mat]);

        mesh = new MeshObject(wall, view, -1);
    }

    mesh->setPosition(rotateCcw(0, 0, -(side & 1 ? roomWidth : roomLength) / 2, side * 90));
    mesh->setRotationVector(QVector3D(0, 1, 0));
    mesh->setRotationAngle(side * 90);

    solid << mesh;
}

void Room::loadDefaultModels() {
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
    floorBuilder.addPane(QSizeF(roomWidth, roomLength));
    floorBuilder.currentNode()->setLocalTransform(floorTrans);
    QGLSceneNode *floorNode = floorBuilder.finalizedSceneNode();
    floorNode->setEffect(QGL::LitModulateTexture2D);
    floor = new MeshObject(floorNode, view, -1);

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(QSizeF(roomWidth, roomLength));
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();

    ceil = new MeshObject(ceilNode, view, -1);
}
