#ifndef MYSCENE_H
#define MYSCENE_H

#include<Qt3D/QGLAbstractScene>

class QGLSceneNode;

class MyScene : public QGLAbstractScene
{
	Q_OBJECT
public:
	explicit MyScene(QObject *parent = 0);
	virtual QList<QObject *> objects() const;
	QGLSceneNode *mainNode() const { return rootNode; }
private:
	QGLSceneNode *rootNode;
};

#endif // MYSCENE_H
