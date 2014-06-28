#include "room.h"
#include "common.h"
#include "directory.h"
#include "imageviewer.h"
#include <Qt3D/QGLPainter>
#include <Qt3D/QGLAbstractScene>
#include <Qt3D/QGLBuilder>

inline QVector3D rotateCcw(qreal x, qreal y, qreal z, qreal angle)
{
    return QQuaternion::fromAxisAndAngle(0, 1, 0, angle).rotatedVector(QVector3D(x, y, z));
}

Room::Room(const QString &fileName)
{
    setFloorAndCeil();

    entryModel.resize(typeNameList.size() + 2);
    // set default model
    for (int i = 2; i < entryModel.size(); ++i)
        if (entryModel.at(i) == NULL)
            entryModel[i] = entryModel.at(1);

    QFile file(configDir + fileName);
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

    /* Arrows, temporary solution */
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

void Room::paintFront(QGLPainter *painter, int animObj, qreal animProg) const
{
    // paint solid models
    for (const MeshInfo &obj : solid) {
        painter->setColor(QColor(Qt::white));
        obj.draw(painter, animObj != -1 && obj.id == animObj ? animProg : 0.0);
    }

    // paint floor and ceil
    floor->draw(painter);
    // walkaround for material issue
    painter->setStandardEffect(QGL::LitMaterial);
    ceil->draw(painter);

    // paint entries
    for (int i = 0; i < frontPage.size(); ++i)
        if (i != pickedEntry)
            paintMesh(painter,
                    entryModel[frontPage[i]], slot[i], i,
                    frontPage[i] == 0 ? &dirAnim : NULL,
                    i == animObj ? animProg : 0.0);

    frontImage->draw(painter);
}

void Room::paintBack(QGLPainter *painter, AnimStage stage) const
{
    // paint solid models
    for (const MeshInfo &obj : solid)
        obj.draw(painter);

    // paint floor and ceil
    if (stage == Entering1 || stage == Entering2) {
        /* Move floor a little bit upper to cover outside items */
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

    // paint entires
    if (stage != Leaving1 && stage != Leaving2)
        for (int i = 0; i < backPage.size(); ++i)
            paintMesh(painter,
                    entryModel[backPage[i]], slot[i], i,
                    backPage[i] == 0 ? &dirAnim : NULL);

    if (stage != Leaving1)
        backImage->draw(painter);

}

void Room::loadFront(Directory *dir)
{
    frontPage = dir->entryTypeList();
    frontImage->setFile(dir->getPlayingFile("image"));
}

void Room::loadBack(Directory *dir)
{
    backPage = dir->entryTypeList();
    backImage->setFile(dir->getPlayingFile("image"));
}

void Room::switchBackAndFront()
{
    frontPage.swap(backPage);
    ImageViewer *tmp = frontImage;
    frontImage = backImage;
    backImage = tmp;
}

void Room::setImage(const QString &fileName)
{
    frontImage->setFile(fileName);
}

void Room::paintPickedEntry(QGLPainter *painter, const QVector3D &delta) const
{
    QMatrix4x4 trans;
    trans.translate(delta);
    trans *= slot[pickedEntry];
    paintMesh(painter,
            entryModel[frontPage[pickedEntry]], trans, -1,
            frontPage[pickedEntry] == 0 ? &dirAnim : NULL);
}

void Room::loadProperty(const QString &property, QTextStream &value)
{
    static QMatrix4x4 slotBase;

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
        floor->setMaterial(palette[floorMat]);
        ceil->setMaterial(palette[ceilMat]);

    } else if (property == "entryModel") {
        loadEntryModel(value);

    } else if (property == "slotGroup") {
        qreal x, y, z, angle, scale;
        value >> x >> y >> z >> angle >> scale;
        slotBase.setToIdentity();
        slotBase.translate(x, y, z);
        slotBase.rotate(angle, 0, 1, 0);

    } else if (property == "slot") {
        qreal x, y, z;
        value >> x >> y >> z;
        slot.append(slotBase);
        slot.back().translate(x, y, z);
    }
}

void Room::loadModel(QTextStream &value)
{
    QString name, type, anim;
    qreal x, y, z, w, angle;
    value >> name >> type >> x >> y >> z >> w >> angle >> anim;

    int id;
    if (type == "-")
        id = -1;
    else if (type == "TrashBin")
        id = TrashBin;
    else if (type == "Image") {
        id = Image;

        // TODO: 
        frontImage = new ImageViewer(30, 20);
        frontImage->setPosition(QVector3D(x, y, z + 1));

        backImage = new ImageViewer(30, 20);
        backImage->setPosition(QVector3D(x, y, z + 1));


    } else if (type == "Door") {
        id = Door;
        doorPos = QVector3D(x, y, z);
        doorAngle = angle;
    }

    QMatrix4x4 trans;
    trans.translate(x, y, z);
    trans.scale(w);
    trans.rotate(angle, 0, 1, 0);

    solid.append(MeshInfo{models[name], trans, id, NULL});

    // load animation info
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
}

void Room::loadEntryModel(QTextStream &value)
{
    QString fileType, modelName;

    value >> fileType >> modelName;
    if (fileType == "DIR") {
        /* FIXME: scale it in .obj file! */
        QMatrix4x4 trans;
        trans.scale(1.3);

        entryModel[0] = models[modelName];
        models[modelName]->setLocalTransform(trans);
        value >> modelName;
        dirAnim.mesh = models[modelName];
        models[modelName]->setLocalTransform(trans);
        qreal x, y, z;
        value >> x >> y >> z;
        dirAnim.center = QVector3D(x, y, z);
        value >> x >> y >> z;
        dirAnim.axis = QVector3D(x, y, z);
        value >> dirAnim.maxAngle;

    } else if (fileType == "DEFAULT") {
        entryModel[1] = models[modelName];

    } else {
        int idx = typeNameList.indexOf(fileType);
        Q_ASSERT(idx != -1);
        entryModel[idx + 2] = models[modelName];
    }
}

// TODO: texture support
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

    floor = floorNode;

    QMatrix4x4 ceilTrans;
    ceilTrans.rotate(90, 1, 0, 0);
    ceilTrans.translate(0, 0, -roomHeight);
    QGLBuilder ceilBuilder;
    ceilBuilder.newSection(QGL::Faceted);
    ceilBuilder.addPane(QSizeF(roomWidth, roomLength));
    ceilBuilder.currentNode()->setLocalTransform(ceilTrans);
    QGLSceneNode *ceilNode = ceilBuilder.finalizedSceneNode();

    ceil = ceilNode;
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

void Room::MeshInfo::draw(QGLPainter *painter, qreal animProg) const
{
    paintMesh(painter, mesh, transform, id, anim, animProg);
}

void Room::paintMesh(QGLPainter *painter,
        QGLSceneNode *mesh, const QMatrix4x4 &trans, int id,
        const AnimInfo *anim, qreal animProg)
{
    painter->modelViewMatrix().push();
    painter->modelViewMatrix() *= trans;

    int prevObjectId = painter->objectPickId();
    painter->setObjectPickId(id);

    if (painter->isPicking() && hoveringId != -1 && hoveringId == id)
        hoveringPickColor = painter->pickColor();

    mesh->draw(painter);

    if (anim)
        anim->draw(painter, animProg);

    painter->modelViewMatrix().pop();
    painter->setObjectPickId(prevObjectId);
}
