#ifndef PICKOBJECT_H
#define PICKOBJECT_H

#include <QtCore/QObject>

class GLView;

class PickObject : public QObject {
    Q_OBJECT
public:
    PickObject(GLView *view, int id);
    ~PickObject();
    int objectId() const;
    void setObjectId(int id);

    void ignoreMuting(bool ignore);
    static void muteObjectId(bool mute);

private:
    GLView *view;
    int id;

    bool ignoringMuting;
    static bool idMuted;
};

#endif
