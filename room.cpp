#include "room.h"
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
    return slotPos.at(i);
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

void Room::MeshInfo::draw(QGLPainter *painter, qreal animProg) const
{
    painter->modelViewMatrix().push();
    int prevId = painter->objectPickId();

    painter->modelViewMatrix() *= transform;
    if (paintingOutline == -1 || id == paintingOutline)
        painter->setObjectPickId(id);
    mesh->draw(painter);

    if (anim) anim->draw(painter, animProg);

    painter->setObjectPickId(prevId);
    painter->modelViewMatrix().pop();
}

void Room::AnimInfo::draw(QGLPainter *painter, qreal animProg) const
{
    if (animProg != 0.0) {
        painter->modelViewMatrix().translate(center);
        painter->modelViewMatrix().rotate(maxAngle * animProg, axis);
        painter->modelViewMatrix().translate(-center);
    }
    mesh->draw(painter);
}

void Room::paintFront(QGLPainter *painter, int animObj, qreal animProg) {
    for (const MeshInfo &obj : solid)
        obj.draw(painter, animObj != -1 && obj.id == animObj ? animProg : 0.0);

    floor->draw(painter);
    // walkaround for material issue
    painter->setStandardEffect(QGL::LitMaterial);
    ceil->draw(painter);

    for (int i = 0; i < entryNum; ++i)
        if (i != pickedEntry) {
            painter->modelViewMatrix().push();
            int prevObjectId = painter->objectPickId();

            painter->modelViewMatrix().translate(slotPos.at(i));
            if (paintingOutline == -1 || i == paintingOutline)
                painter->setObjectPickId(i);

            fileModel[frontPage[i]]->draw(painter);

            if (frontPage[i] == "dir")
                dirAnim.draw(painter, i == animObj ? animProg : 0.0);

            painter->modelViewMatrix().pop();
            painter->setObjectPickId(prevObjectId);
        }
}

void Room::paintBack(QGLPainter *painter, int stage) {
    for (const MeshInfo &obj : solid)
        obj.draw(painter);

    if (stage == Entering1 || stage == Entering2) {
        painter->modelViewMatrix().push();
        painter->modelViewMatrix().translate(0, 0.001, 0);
        floor->draw(painter);
        painter->modelViewMatrix().pop();
    } else {
        floor->draw(painter);
        // walkaround for material issue
        painter->setStandardEffect(QGL::LitMaterial);
        ceil->draw(painter);
    }

    if (stage != Leaving1 && stage != Leaving2)
        for (int i = 0; i < backEntryNum; ++i) {
            painter->modelViewMatrix().push();
            int prevObjectId = painter->objectPickId();

            painter->modelViewMatrix().translate(slotPos.at(i));
            painter->setObjectPickId(-1);

            fileModel[backPage[i]]->draw(painter);

            if (backPage[i] == "dir")
                dirAnim.draw(painter);

            painter->modelViewMatrix().pop();
            painter->setObjectPickId(prevObjectId);
        }
}

void Room::paintPickedEntry(QGLPainter *painter, const QVector3D &delta)
{
    painter->modelViewMatrix().push();
    painter->modelViewMatrix().translate(delta);
    painter->modelViewMatrix().translate(slotPos.at(pickedEntry));

    fileModel[frontPage[pickedEntry]]->draw(painter);
    if (frontPage[pickedEntry] == "dir")
        dirAnim.draw(painter);

    painter->modelViewMatrix().pop();
}

Room::Room(const QString &name, GLView *, void *) : slotNum(0)
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

    model = QGLAbstractScene::loadScene(dataDir + QString("leftarrow.obj"));
    model->mainNode()->setMaterial(palette["tmp2"]);

    QMatrix4x4 trans;
    trans.translate(-50, 90, -roomLength / 2);
    trans.scale(0.4);

    solid.append(MeshInfo{model->mainNode(), trans, LeftArrow, NULL});

    model = QGLAbstractScene::loadScene(dataDir + QString("rightarrow.obj"));
    model->mainNode()->setMaterial(palette["tmp2"]);

    trans.setToIdentity();
    trans.translate(50, 90, -roomLength / 2);
    trans.scale(0.4);

    solid.append(MeshInfo{model->mainNode(), trans, RightArrow, NULL});
}

void Room::loadModel(QTextStream &value)
{
    QString name, anim;
    int type;
    qreal x, y, z, w, angle;
    value >> name >> type >> x >> y >> z >> w >> angle >> anim;

    if (type == 3) {
        doorPos = QVector3D(x, y, z);
        doorAngle = angle;
    }

    QMatrix4x4 trans;
    trans.translate(x, y, z);
    trans.scale(w);
    trans.rotate(angle, 0, 1, 0);

    solid.append(MeshInfo{models[name], trans, id[type], NULL});

    if (anim != "-") {
        AnimInfo *animInfo = new AnimInfo;
        animInfo->mesh = models[anim];
        value >> x >> y >> z;
        animInfo->center = QVector3D(x, y, z);
        value >> x >> y >> z;
        animInfo->axis = QVector3D(x, y, z);
        value >> animInfo->maxAngle;
        solid.last().anim = animInfo;
    }

    if (type == 1)
        loadContainer("shelf", QVector3D(0, 0, -80));
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
        //floor->mesh()->setMaterial(palette[floorMat]);
        //ceil->mesh()->setMaterial(palette[ceilMat]);
        floor->setMaterial(palette[floorMat]);
        ceil->setMaterial(palette[ceilMat]);

    } else if (property == "slot") { }
}

void Room::loadContainer(const QString &name, const QVector3D &basePos) {
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

    dirAnim.mesh = dirAnimModel;
    dirAnim.center = animCenter;
    dirAnim.axis = animVector;
    dirAnim.maxAngle = 90.0;

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

    fileModel.insert("dir", dirSolidModel);

    stream >> n;
    slotNum += n;

    for (int i = 0; i < n; ++i) {
        qreal x, y, z;
        stream >> x >> y >> z;
        slotPos.append(QVector3D(x, y, z) + basePos);
    }

    file.close();
}

/* FIXME: material is broken */
void Room::loadWall(QTextStream &value) {
    int side;
    qreal l, r, h;
    QString mat;
    value >> side >> l >> r >> h >> mat;

    QGLSceneNode *mesh;

    qreal w = side & 1 ? roomLength : roomWidth;
    qDebug() << side << w;
    if (l == -1) {
        QGLBuilder roomBuilder;
        roomBuilder.newSection(QGL::Faceted);
        roomBuilder.addPane(QSizeF(w, roomHeight));
        QGLSceneNode *pane = roomBuilder.finalizedSceneNode();
        pane->setMaterial(palette[mat]);
        pane->setPosition(QVector3D(0, roomHeight * 0.5, 0));

        mesh = pane;
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

        mesh = wall;
    }

    QMatrix4x4 trans;
    trans.translate(rotateCcw(0, 0, -(side & 1 ? roomWidth : roomLength) / 2, side * 90));
    trans.rotate(side * 90, 0, 1, 0);

    solid.append(MeshInfo{mesh, trans, -1, NULL});
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

    //floor = new MeshObject(floorNode, view, -1);
    floor = floorNode;

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(QSizeF(roomWidth, roomLength));
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();

    //ceil = new MeshObject(ceilNode, view, -1);
    ceil = ceilNode;
}

void Room::loadDir(Directory *, bool back) {
    if (back) {
        backEntryNum = dir->count();
        backPage = dir->entryTypeList();
    } else {
        entryNum = dir->count();
        frontPage = dir->entryTypeList();
    }
}

void Room::pushToFront() {
    entryNum = backEntryNum;
    frontPage = backPage;
}

void Room::clearBack()
{
    backPage.clear();
}
