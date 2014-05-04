#include "pickobject.h"
#include <Qt3D/QGLView>

PickObject::PickObject(QGLView *view, int id) :
    QObject(view), view(view), id(id)
{
    if (view && id != -2) view->registerObject(id, this);
}

PickObject::~PickObject() {
    //if (view && di != -2) view->deregisterObject(id);
}

int PickObject::objectId() const { return id; }
