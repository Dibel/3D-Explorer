#ifndef MY3DVIEW_H
#define MY3DVIEW_H

#include <Qt3D/QGLView>
#include <Qt3D/QGLPickNode>
#include "myscene.h"

class QGLSceneNode;

class My3DView : public QGLView
{
	Q_OBJECT
public:
	My3DView(QWindow *parent = 0);
	~My3DView();
protected:
	void initializeGL(QGLPainter * painter);
	void paintGL(QGLPainter *painter);
	void mousePressEvent(QMouseEvent *e);

signals:

public slots:
	void objectPicked();
private:
	void registerPickableNodes();
	QGLPickNode *myPickNode;
	QGLSceneNode *myNode;
	MyScene *myScene;
};

#endif // MY3DVIEW_H
