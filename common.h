#ifndef COMMON_H
#define COMMON_H

#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>

const int roomSize = 80;
const int roomHeight = 120;
const int eyeHeight = 50;
const qreal boxScale = 0.05;

const int MaxBoxId = 100;
const int StartImageId = 200;

enum AnimStage : int { NoAnim = 0, Entering1, Entering2, Leaving1, Leaving2, Leaving3, TurningLeft, TurningRight };
enum { MaxBox = MaxBoxId, TrashBin, Door, LeftArrow, RightArrow, Picture = StartImageId };

inline QVector3D rotateCcw(QVector3D vec, qreal angle) {
    return QQuaternion::fromAxisAndAngle(0, 1, 0, angle).rotatedVector(vec);
}

inline QVector3D rotateCcw(qreal x, qreal y, qreal z, qreal angle) {
    return rotateCcw(QVector3D(x, y, z), angle);
}
#endif
