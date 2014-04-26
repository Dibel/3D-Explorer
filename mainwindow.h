#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "view.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

signals:

public slots:
    void changeDir(QDir inputDir);
private:
    View *m_view;
    QWidget *m_container;
};

#endif // MAINWINDOW_H
