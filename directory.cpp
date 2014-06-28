#include "directory.h"
#include "common.h"
#include <QtWidgets/QMessageBox>

Directory::Directory(int size) : QDir(), playingFiles(typeNameList.size())
{
#ifdef Q_OS_WIN
    isThisPC = false;
#endif
    setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    setSorting(QDir::IgnoreCase);

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

QString Directory::playFile(int index, const QString &assumedType)
{
    index += offset;
    if (typeList.at(index) < 2)
        return QString();
    if (typeNameList[typeList.at(index) - 2] != assumedType)
        return QString();

    playingFiles[typeList.at(index) - 2] = index;
    return absoluteFilePath(index);
}

QString Directory::playNext(const QString &typeName)
{
    int type = typeNameList.indexOf(typeName);
    Q_ASSERT(type != -1);

    int index = typeList.indexOf(type + 2, playingFiles.at(type) + 1);
    if (index == -1)
        index = typeList.indexOf(type + 2);
    if (index == -1)
        return QString();

    playingFiles[type] = index;
    return absoluteFilePath(index);
}

QString Directory::playPrev(const QString &typeName)
{
    int type = typeNameList.indexOf(typeName);
    Q_ASSERT(type != -1);

    int index = typeList.lastIndexOf(type + 2, playingFiles.at(type) - 1);
    if (index == -1)
        index = typeList.lastIndexOf(type + 2);
    if (index == -1)
        return QString();

    playingFiles[type] = index;
    return absoluteFilePath(index);
}

QString Directory::getPlayingFile(const QString &type)
{
    int index = playingFiles.at(typeNameList.indexOf(type));
    if (index >= 0) {
        return absoluteFilePath(index);
    } else {
        return QString();
    }
}

void Directory::update()
{
    offset = 0;

#ifdef Q_OS_WIN
    if (isThisPC) {
        entryList.clear();
        for (auto drive : QDir::drives())
            entryList << drive.filePath();
        playingFiles.fill(-1);
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

        if (begin != end) {
            playingFiles[i] = begin;
            for (int j = begin; j < end; ++j)
                typeList[j] = i + 2;
            begin = end;
        } else
            playingFiles[i] = -1;
    }

    entryList << QDir::entryList(QDir::Files);
    entryList.removeDuplicates();
    for (int i = begin; i < entryList.size(); ++i)
        typeList[i] = 1;
}
