#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtCore/QDir>
#include <QtGui/QImage>

class Directory : public QDir {
public:
    Directory();

#ifdef Q_OS_WIN
    QString absolutePath();
    bool cd(const QString &dirName);
    bool cdUp();
    QStringList entryList();
#endif
    int count();
    int countDir();
    void updateLists();
    QString getImage();
    QString getNextImage();

private:
#ifdef Q_OS_WIN
    bool isThisPc;
#endif

    QStringList bufferedEntryList;
    int dirCnt;

    QStringList imageList;
    int curImageIdx;
};

#endif
