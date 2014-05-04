#include "directory.h"
#include <QtCore/QDebug>

Directory::Directory() : QDir(), pageIndex(0) {
#ifdef Q_OS_WIN
    isThisPC = false;
#endif
    setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    setSorting(QDir::DirsFirst | QDir::IgnoreCase);
    updateLists();
}

void Directory::setPageSize(int size) { pageSize = size; }

#ifdef Q_OS_WIN
QString Directory::absolutePath() {
    return isThisPC ? "This PC" : QDir::absolutePath();
}
#endif

bool Directory::cd(const QString &dirName) {
#ifdef Q_OS_WIN
    if (isThisPC) {
        setPath(dirName);
        if (exists()) isThisPC = false;
        return !isThisPC;
    }
#endif
    bool success = QDir::cd(dirName);
    if (success) updateLists();
    return success;
}

bool Directory::cdUp() {
#ifdef Q_OS_WIN
    if (isThisPC) return false;
    if (isRoot()) { isThisPC = true; return true; }
#endif
    bool success = QDir::cdUp();
    if (success) updateLists();
    return success;
}

uint Directory::count() {
#ifdef Q_OS_WIN
    if (isThisPC) return QDir::drives().size();
#endif
    return page.size();
}

uint Directory::countDir() {
    int cnt = QDir::entryList(QDir::Dirs | QDir::NoDotAndDotDot).size();
    cnt -= pageSize * pageIndex;
    return cnt < 0 ? 0 : cnt;
}

QStringList Directory::entryList() { return page; }

QString Directory::entry(int index) { return page[index]; }

void Directory::nextPage() {
    if (++pageIndex * pageSize >= QDir::count()) --pageIndex;
    page = QDir::entryList().mid(pageIndex * pageSize, pageSize);
}

void Directory::prevPage() {
    if (pageIndex > 0) --pageIndex;
    page = QDir::entryList().mid(pageIndex * pageSize, pageSize);
}

QString Directory::getImage() {
    return imageList.isEmpty() ? ":/model/photo.png"
        : absoluteFilePath(imageList[imageIndex]);
}

QString Directory::getNextImage() {
    if (imageList.isEmpty()) return ":/model/photo.png";
    if (++imageIndex >= imageList.size())
        imageIndex -= imageList.size();
    return absoluteFilePath(imageList[imageIndex]);
}

void Directory::updateLists() {
    qDebug() << "updating list";
    pageIndex = 0;
#ifdef Q_OS_WIN
    if (isThisPC) {
        page.clear();
        for (auto drive : QDir::drives()) page << drive.filePath();
        /* warning: assume drives are less than slots */
    } else
#endif
    page = QDir::entryList().mid(pageIndex * pageSize, pageSize);

    static const QStringList imageFilter = {
        "*.bmp", "*.jpg", "*.jpeg", "*.gif", "*.png" };
    imageList = QDir::entryList(imageFilter, QDir::Files);
    imageIndex = 0;
}
