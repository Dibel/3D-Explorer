#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtCore/QDir>
#include <QtGui/QImage>

class Directory : public QDir {
public:
    Directory();
    void setPageSize(int size);

    bool cd(int index);
    bool cdUp();

    void nextPage();
    void prevPage();

    void refresh();

    bool remove(int index);

    inline int count() const
    {
        return qMin(entryList.size() - offset, pageSize);
    }

    inline int countDir() const
    {
        return qMax(dirCnt - offset, 0);
    }

    inline QString entry(int index) const
    {
        return entryList.at(offset + index);
    }

    inline QVector<int> entryTypeList() const
    {
        return typeList.mid(offset, pageSize);
    }

#ifdef Q_OS_WIN
    QString absolutePath() const;
#endif

    QString getImage();
    QString getNextImage();

private:
    void update();

#ifdef Q_OS_WIN
    bool isThisPC;
#endif

    int pageSize = -1;
    int offset = 0;
    int dirCnt = 0;

    QStringList entryList;
    QVector<int> typeList;

    QStringList imageList;
    int imageIndex;
};

#endif
