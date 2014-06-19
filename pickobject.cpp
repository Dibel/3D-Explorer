#include "pickobject.h"
#include "lib/glview.h"
//#include <Qt3D/QGLView>

int PickObject::outlineId = -1;

PickObject::PickObject(GLView *view, int id) :
    QObject(view), view(view), id(id)
{
    if (view && id != -2) view->registerObject(id, this);
}

PickObject::~PickObject() {
    //if (view && di != -2) view->deregisterObject(id);
}

//int PickObject::objectId() const { return id; }
int PickObject::objectId() const { return outlineId == -1 || outlineId == id ? id : -1; }

void PickObject::setObjectId(int newId) { id = newId; }

void PickObject::paintOutline(int id) { outlineId = id; }
