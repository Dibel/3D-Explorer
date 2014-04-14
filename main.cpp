#include "mainwindow.h"
#include "my3dview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	My3DView view;
	view.resize(800,600);
	view.show();
	//MainWindow w;
	//w.show();

	return a.exec();
}
