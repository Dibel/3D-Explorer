#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtCore/QDir>
#include <QtGui/QImage>

class Directory : public QDir {
public:
    Directory();
    void setPageSize(int size);

#ifdef Q_OS_WIN
    QString absolutePath();
#endif

    bool cd(const QString &dirName);
    bool cdUp();
    uint count();
    uint countDir();
    QStringList entryList();
    QString entry(int index);

    void nextPage();
    void prevPage();

    QString getImage();
    QString getNextImage();

private:
    void updateLists();

#ifdef Q_OS_WIN
    bool isThisPC;
#endif

    int pageSize;
    QStringList page;
    int pageIndex;

    QStringList imageList;
    int imageIndex;
};

#endif
