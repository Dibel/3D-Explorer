#include "pickobject.h"
#include <Qt3D/QGLView>

bool PickObject::idMuted = false;

PickObject::PickObject(QGLView *view, int id) :
    QObject(view), view(view), id(id), ignoringMuting(false)
{
    if (view && id != -2) view->registerObject(id, this);
}

PickObject::~PickObject() {
    //if (view && di != -2) view->deregisterObject(id);
}

//int PickObject::objectId() const { return id; }
int PickObject::objectId() const { return idMuted && !ignoringMuting ? -1 : id; }

void PickObject::setObjectId(int newId) { id = newId; }

void PickObject::ignoreMuting(bool ignore) { ignoringMuting = ignore; }

void PickObject::muteObjectId(bool mute) { idMuted = mute; }
