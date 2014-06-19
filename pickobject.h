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

    static void paintOutline(int id);

private:
    GLView *view;
    int id;

    static int outlineId;
};

#endif
