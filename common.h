#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QSet>
#include <QtGui/QColor>
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>

class QGLMaterial;
class QGLSceneNode;
class QTextStream;
class GLView;
class Room;

#ifdef Q_OS_WIN
const char configDir[] = "./config/";
const char dataDir[] = "./data/";
#else
const char configDir[] = "config/";
const char dataDir[] = "data/";
#endif

const int defaultWindowWidth = 800;
const int defaultWindowHeight = 600;

const int MaxBoxId = 100;
const int StartImageId = 200;

enum AnimStage : int { NoAnim = 0, Entering1, Entering2, Leaving1, Leaving2, Leaving3, TurningLeft, TurningRight };
enum { MaxBox = MaxBoxId, TrashBin, Door, LeftArrow, RightArrow, Image, ImagePrevBtn, ImageNextBtn };

extern int roomWidth, roomLength, roomHeight, eyeHeight;
extern qreal boxScale;

extern QHash<QString, QGLMaterial*> palette;
extern QHash<QString, QGLSceneNode*> models;
extern QHash<QString, Room*> rooms;

extern QList<QStringList> typeFilters;
extern QHash<QString, QString> fileType;
extern QStringList typeList;

extern QHash<QString, int> extToIndex;

extern GLView *view;

static const int id[4] = { -1, -1, TrashBin, Door };

extern int paintingOutline;
extern QColor hoveringPickColor;

inline QVector3D rotateCcw(QVector3D vec, qreal angle) {
    return QQuaternion::fromAxisAndAngle(0, 1, 0, angle).rotatedVector(vec);
}

inline QVector3D rotateCcw(qreal x, qreal y, qreal z, qreal angle) {
    return rotateCcw(QVector3D(x, y, z), angle);
}

#endif
