#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QHash>
#include <QtGui/QColor>

class QGLMaterial;
class QGLSceneNode;
class Room;

#ifdef Q_OS_WIN
const char configDir[] = "./config/";
const char dataDir[] = "./data/";
#else
const char configDir[] = "config/";
const char dataDir[] = "data/";
#endif

extern int windowWidth;
extern int windowHeight;

extern int hoveringId;
extern QColor hoveringPickColor;

extern int roomWidth, roomLength, roomHeight, eyeHeight;
extern qreal boxScale;

extern QHash<QString, QGLMaterial*> palette;
extern QHash<QString, QGLSceneNode*> models;
extern QHash<QString, Room*> rooms;

extern QList<QStringList> typeFilters;
extern QStringList typeNameList;
extern QHash<QString, int> extToIndex;

enum AnimStage : int { NoAnim = 0, Entering1, Entering2, Leaving1, Leaving2, Leaving3, TurningLeft, TurningRight };
enum { MaxBox = 100, TrashBin, Door, LeftArrow, RightArrow, Image, ImagePrevBtn, ImageNextBtn };

#endif
