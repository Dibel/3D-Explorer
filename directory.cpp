#include "directory.h"
#include "common.h"
#include <QtCore/QDebug>
#include <QtWidgets/QMessageBox>

Directory::Directory() : QDir()
{
#ifdef Q_OS_WIN
    isThisPC = false;
#endif
    setSorting(QDir::IgnoreCase);
}

void Directory::setPageSize(int size)
{
    pageSize = size;
    update();
}

#ifdef Q_OS_WIN
QString Directory::absolutePath() const
{
    return isThisPC ? "This PC" : QDir::absolutePath();
}
#endif

bool Directory::cd(int index)
{
#ifdef Q_OS_WIN
    if (isThisPC) {
        setPath(entryList.at(offset + index));
        if (exists()) isThisPC = false;
        update();
        return !isThisPC;
    }
#endif

    bool success = QDir::cd(entryList.at(offset + index));
    if (success) update();
    return success;
}

bool Directory::cdUp()
{
#ifdef Q_OS_WIN
    if (isThisPC) return false;
    if (isRoot()) { isThisPC = true; update(); return true; }
#endif

    bool success = QDir::cdUp();
    if (success) update();
    return success;
}

void Directory::refresh()
{
    int tmp = offset;
    QDir::refresh();
    update();
    offset = tmp;
    while (offset >= (int)QDir::count())
        offset -= pageSize;
    if (offset < 0) offset = 0;
}

bool Directory::remove(int index)
{
#ifdef Q_OS_WIN
    if (isThisPC) return false;
#endif

    if (QMessageBox::question(NULL, "Confirm", "Delete it?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes)
        return false;

    const QString &fileName = entryList.at(offset + index);
    if (offset + index < dirCnt
            ? QDir(absoluteFilePath(fileName)).removeRecursively()
            : QDir::remove(fileName))
    {
        refresh();
        return true;
    }

    return false;
}

void Directory::nextPage()
{
    offset += pageSize;
    if (offset >= (int)QDir::count())
        offset -= pageSize;
}

void Directory::prevPage()
{
    if (offset > 0)
        offset -= pageSize;
}

QString Directory::getImage() {
    return imageList.isEmpty() ? ":/data/default.png"
        : absoluteFilePath(imageList[imageIndex]);
}

QString Directory::getNextImage() {
    if (imageList.isEmpty()) return ":/data/default.png";
    if (++imageIndex >= imageList.size())
        imageIndex -= imageList.size();
    return absoluteFilePath(imageList[imageIndex]);
}

void Directory::update()
{
    offset = 0;

#ifdef Q_OS_WIN
    if (isThisPC) {
        for (auto drive : QDir::drives())
            entryList << drive.filePath();
        imageList.clear();
        imageIndex = 0;
        return;
    }
#endif

    entryList.clear();
    typeList.resize(QDir::count());

    entryList << QDir::entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    dirCnt = entryList.size();
    for (int i = 0; i < dirCnt; ++i)
        typeList[i] = 0;

    int begin = dirCnt;
    int end;

    for (int i = 0; i < typeFilters.size(); ++i) {
        entryList << QDir::entryList(typeFilters.at(i), QDir::Files);

        end = entryList.size();
        for (int j = begin; j < end; ++j)
            typeList[j] = i + 2;
        begin = end;
    }

    entryList << QDir::entryList(QDir::Files);
    entryList.removeDuplicates();
    for (int i = begin; i < entryList.size(); ++i)
        typeList[i] = 1;

    imageList = QDir::entryList(typeFilters[0], QDir::Files);
    imageIndex = 0;
}
