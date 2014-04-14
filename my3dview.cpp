#include "my3dview.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLCube>
#include <QtCore/qpropertyanimation.h>
#include <QMessageBox>

My3DView::My3DView(QWindow *parent)
	: QGLView(parent)
{
	QGLBuilder builder;
	builder << QGL::Smooth;
	builder << QGLCube(1.5f);
	myNode=builder.finalizedSceneNode();
	myNode->setParent(this);
	myScene=new MyScene(this);
	myScene->mainNode()->addNode(myNode);
	myScene->setPickable(true);
	setOption(QGLView::ObjectPicking, true);
	registerPickableNodes();
}

My3DView::~My3DView()
{
	delete myNode;
	delete myScene;
	delete myPickNode;
}

void My3DView::initializeGL(QGLPainter *painter)
{

}

void My3DView::paintGL(QGLPainter *painter)
{
	myNode->draw(painter);
}

void My3DView::mousePressEvent(QMouseEvent *e)
{
	QGLView::mousePressEvent(e);
}

void My3DView::objectPicked()
{
	QMessageBox::information(NULL,"test","test");
}

void My3DView::registerPickableNodes()
{
	myScene->generatePickNodes();
	QList <QGLPickNode *> pickList=myScene->pickNodes();
	foreach(QGLPickNode *node,pickList)
	{
		connect(node,SIGNAL(clicked()),this,SLOT(objectPicked()));
		registerObject(node->id(),node);
	}
}
