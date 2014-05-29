#ifndef PICKOBJECT_H
#define PICKOBJECT_H

#include <QtCore/QObject>

class QGLView;

class PickObject : public QObject {
    Q_OBJECT
public:
    PickObject(QGLView *view, int id);
    ~PickObject();
    int objectId() const;
    void setObjectId(int id);

    void ignoreMuting(bool ignore);
    static void muteObjectId(bool mute);

private:
    QGLView *view;
    int id;

    bool ignoringMuting;
    static bool idMuted;
};

#endif
