#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    m_view = new View(QDir());
    m_container = QWidget::createWindowContainer(m_view);
    this->setCentralWidget(m_container);
    this->resize(800,600);
    connect(m_view,SIGNAL(newDir(QDir)),this,SLOT(changeDir(QDir)));
}

void MainWindow::changeDir(QDir inputDir)
{
    //delete this->m_view;
    //delete this->m_container;
    m_view->setVisible(false);
    m_container->setDisabled(true);
    m_view = new View(inputDir);
    m_view->resize(800,600);
    m_container = QWidget::createWindowContainer(m_view);
    this->setCentralWidget(m_container);
}
