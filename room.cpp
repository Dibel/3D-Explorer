#include "room.h"
#include "meshobject.h"
#include "common.h"
#include "directory.h"
#include "lib/glview.h"
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <Qt3D/QGLMaterial>
#include <QtCore/QFile>

QVector3D Room::getEntryPos(int i) const
{
    return entry.at(i)->position();
}

QVector3D Room::getDoorPos() const
{
    return door->position();
}

qreal Room::getDoorAngle() const
{
    return door->rotationAngle();
}

inline void setAllMaterial(QGLSceneNode *node, QGLMaterial *mat)
{
    int index = node->palette()->addMaterial(mat);
    node->setMaterialIndex(index);
    node->setEffect(QGL::LitModulateTexture2D);
    for (QGLSceneNode *child : node->allChildren()) {
        child->setMaterialIndex(index);
        child->setEffect(QGL::LitModulateTexture2D);
    }
}

void Room::paintFront(QGLPainter *painter, int animObj, qreal animProg) {
    for (MeshObject *obj : solid) {
        if (obj->objectId() == animObj) {
            obj->setAnimAngle(90.0 * animProg);
            obj->draw(painter);
            obj->setAnimAngle(0);
        } else
            obj->draw(painter);
    }

    floor->draw(painter);
    ceil->draw(painter);

    if (animObj >= 0 && animObj < entryNum)
        entry[animObj]->setAnimAngle(90.0 * animProg);

    for (int i = 0; i < entryNum; ++i)
        if (i != pickedEntry)
            entry[i]->draw(painter);

    if (animObj >= 0 && animObj < entryNum)
        entry[animObj]->setAnimAngle(0);
}

void Room::paintBack(QGLPainter *painter, int stage) {
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
        for (int i = 0; i < backEntryNum; ++i)
            backEntry.at(i)->draw(painter);
}

void Room::paintPickedEntry(QGLPainter *painter, const QVector3D &delta)
{
    painter->modelViewMatrix().push();
    painter->modelViewMatrix().translate(delta);
    entry.at(pickedEntry)->draw(painter);
    painter->modelViewMatrix().pop();
}


Room::Room(const QString &name, GLView *, void *)
    : slotNum(0)
{
    setFloorAndCeil();

    QFile file(configDir + name + ".conf");
    file.open(QIODevice::ReadOnly);
    QString line, property;
    QTextStream stream;
    stream.setString(&line, QIODevice::ReadOnly);

    while (!file.atEnd()) {
        line = file.readLine();
        if (line.isEmpty() || line[0] == '#' || line[0] == '\n' || (line[0] == '\r' && line[1] == '\n')) continue;
        if (line[0] == '[') {
            property = line.mid(1, line.indexOf(']') - 1);
        } else {
            stream.seek(0);
            loadProperty(property, stream);
        }
    }

    file.close();

    /* arrows */
    QGLAbstractScene *model;
    MeshObject *mesh;

    model = QGLAbstractScene::loadScene(dataDir + QString("leftarrow.obj"));
    model->mainNode()->setMaterial(palette["tmp2"]);
    mesh = new MeshObject(model->mainNode(), view, LeftArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(-50, 90, -roomLength / 2));
    solid << mesh;

    model = QGLAbstractScene::loadScene(dataDir + QString("rightarrow.obj"));
    model->mainNode()->setMaterial(palette["tmp2"]);
    mesh = new MeshObject(model->mainNode(), view, RightArrow);
    mesh->setScale(0.4);
    mesh->setPosition(QVector3D(50, 90, -roomLength / 2));
    solid << mesh;
}

void Room::loadProperty(const QString &property, QTextStream &value) {
    if (property == "model") {
        loadModel(value);

    } else if (property == "cdup") {
        qreal x, y, z, angle;
        value >> x >> y >> z >> angle;
        outPos = QVector3D(x, y, z);
        outAngle = angle;

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
    int type, recursive, isGlobal;
    value >> name >> isGlobal >> type >> x >> y >> z >> w >> angle >> mat >> recursive;

    MeshObject *mesh;

    if (isGlobal)
        mesh = new MeshObject(models[name], view, id[type]);
    else {
        QGLAbstractScene *model = QGLAbstractScene::loadScene(dataDir + name + ".obj");
        model->setParent(view);
    
        if (mat != "-") {
            if (recursive)
                setAllMaterial(model->mainNode(), palette[mat]);
            else
                model->mainNode()->setMaterial(palette[mat]);
        }
    
        mesh = new MeshObject(model->mainNode(), view, id[type]);
    }

    mesh->setPosition(QVector3D(x, y, z));
    mesh->setScale(w);
    mesh->setRotationVector(QVector3D(0, 1, 0));
    mesh->setRotationAngle(angle);

    QFile file2;
    QTextStream stream2;
    QGLAbstractScene *model;

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
            door = mesh;
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
    qreal x, y, z;
    QString modelName;
    QGLAbstractScene *scene;

    stream >> modelName;

    qreal scale;
    stream >> scale;
    QMatrix4x4 trans;
    trans.scale(scale);

    scene = QGLAbstractScene::loadScene(dataDir + modelName + ".obj");
    scene->setParent(view);
    dirSolidModel = scene->mainNode();
    dirSolidModel->setLocalTransform(trans);

    scene = QGLAbstractScene::loadScene(dataDir + modelName + "_anim.obj");
    scene->setParent(view);
    dirAnimModel = scene->mainNode();
    dirAnimModel->setLocalTransform(trans);

    stream >> x >> y >> z; QVector3D animVector(x, y, z);
    stream >> x >> y >> z; QVector3D animCenter(x, y, z);

    stream >> modelName;

    stream >> scale;
    trans.setToIdentity();
    trans.scale(scale);

    scene = QGLAbstractScene::loadScene(dataDir + modelName + ".obj");
    qDebug() << dataDir + modelName + ".obj";
    scene->setParent(view);
    fileModel.insert("default", scene->mainNode());

    for (const QString &type : typeList) {
        scene = QGLAbstractScene::loadScene(dataDir + modelName + '_' + type + ".obj");
        scene->setParent(view);
        fileModel.insert(type, scene->mainNode());
    }

    stream >> n;
    slotNum += n;

    for (int i = 0; i < n; ++i) {
        qreal x, y, z;
        stream >> x >> y >> z;
        MeshObject *obj = new MeshObject(dirSolidModel, view, entry.size());
        obj->setAnimVector(animVector);
        obj->setAnimCenter(animCenter);
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

void Room::setFloorAndCeil() {
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

inline QString getFileType(const QString &fileName) {
    int p = fileName.lastIndexOf(".");
    if (p == -1) return "default";
    QString ext = fileName.mid(p + 1);

    auto iter = fileType.find(ext);
    if (iter != fileType.end())
        return iter.value();
    else
        return "default";
}

void Room::loadDir(Directory *dir, bool back) {
    QVector<MeshObject*> &boxes = back ? backEntry : entry;

    for (int i = 0; i < dir->count(); ++i) {
        boxes[i]->setPickType(MeshObject::Normal);
        boxes[i]->setObjectName(dir->entry(i));
        boxes[i]->setMesh(i < dir->countDir() ? dirSolidModel : fileModel[getFileType(dir->entry(i))], i < dir->countDir() ? dirAnimModel : NULL);
    }

    for (int i = dir->count(); i < slotNum; ++i) {
        boxes[i]->setPickType(MeshObject::Anchor);
        boxes[i]->setObjectName(QString());
        boxes[i]->setMesh(dirSolidModel, dirAnimModel);
    }

    if (back)
        backEntryNum = dir->count();
    else
        entryNum = dir->count();
}

void Room::pushToFront() {
    for (int i = 0; i < entry.size(); ++i) {
        entry[i]->setPickType(backEntry[i]->pickType());
        entry[i]->setObjectName(backEntry[i]->objectName());
        entry[i]->setMesh(backEntry[i]->mesh(), backEntry[i]->animMesh());
    }
    entryNum = backEntryNum;
}

void Room::clearBack() {
    for (MeshObject *obj : backEntry)
        obj->setPickType(MeshObject::Anchor);
}
