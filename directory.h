#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtCore/QDir>
#include <QtGui/QImage>

class Directory : public QDir {
public:
    Directory();
    void setPageSize(int size);

#ifdef Q_OS_WIN
    QString absolutePath() const;
#endif

    bool cd(const QString &dirName);
    bool cd(int index);
    bool cdUp();
    int count() const;
    int countDir() const;
    QStringList entryList() const;
    QString entry(int index) const;
    void refresh();
    bool remove(int index);

    void nextPage();
    void prevPage();

    QString getImage();
    QString getNextImage();

private:
    void update();

#ifdef Q_OS_WIN
    bool isThisPC;
#endif

    int pageSize;
    QStringList page;
    int pageIndex;
    QStringList fullEntryList;
    QStringList imageList;
    int imageIndex;
};

#endif
