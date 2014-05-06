#ifndef PICKOBJECT_H
#define PICKOBJECT_H

#include <QtCore/QObject>

class QGLView;

const int MaxBoxId = 100;
const int StartImageId = 200;

class PickObject : public QObject {
    Q_OBJECT
public:
    PickObject(QGLView *view, int id);
    ~PickObject();
    int objectId() const;
private:
    int id;
    QGLView *view;
};

#endif