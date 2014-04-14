#include "myscene.h"

MyScene::MyScene(QObject *parent)
	:QGLAbstractScene(parent),rootNode(new QGLSceneNode(this))
{

}

QList<QObject *> MyScene::objects() const
{
	QList<QGLSceneNode *> children = rootNode->allChildren();
	QList<QObject *> objects;
	for (int index = 0; index < children.size(); ++index)
		objects.append(children.at(index));
	return objects;
}


