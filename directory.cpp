#include "directory.h"
#include "common.h"
#include <QtCore/QDebug>
#include <QtWidgets/QMessageBox>

Directory::Directory() : QDir(), pageIndex(0) {
#ifdef Q_OS_WIN
    isThisPC = false;
#endif
    setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Type);
}

void Directory::setPageSize(int size) {
    pageSize = size;
    update();
}

#ifdef Q_OS_WIN
QString Directory::absolutePath() const{
    return isThisPC ? "This PC" : QDir::absolutePath();
}
#endif

bool Directory::cd(const QString &dirName) {
#ifdef Q_OS_WIN
    if (isThisPC) {
        setPath(dirName);
        if (exists()) isThisPC = false;
        update();
        return !isThisPC;
    }
#endif
    bool success = QDir::cd(dirName);
    if (success) update();
    return success;
}

bool Directory::cd(int index)
{
    return cd(page.at(index));
}

bool Directory::cdUp() {
#ifdef Q_OS_WIN
    if (isThisPC) return false;
    if (isRoot()) { isThisPC = true; update(); return true; }
#endif
    bool success = QDir::cdUp();
    if (success) update();
    return success;
}

int Directory::count() const {
#ifdef Q_OS_WIN
    if (isThisPC) return QDir::drives().size();
#endif
    return page.size();
}

int Directory::countDir() const {
    int cnt = QDir::entryList(QDir::Dirs | QDir::NoDotAndDotDot).size();
    cnt -= pageSize * pageIndex;
    return cnt < 0 ? 0 : cnt;
}

QStringList Directory::entryList() const { return page; }

QString Directory::entry(int index) const { return page[index]; }

void Directory::refresh() {
    int pageIndexBackup = pageIndex;
    QDir::refresh();
    update();
    pageIndex = pageIndexBackup;
    while (pageIndex * pageSize >= (int)QDir::count())
        --pageIndex;
    if (pageIndex < 0) pageIndex = 0;
}

bool Directory::remove(int index) {
#ifdef Q_OS_WIN
    if (isThisPC) return false;
#endif
    //qDebug() << "try to remove file";
    //return false;

    if (QMessageBox::question(NULL, "Confirm", "Delete it?", QMessageBox::Yes|QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return false;
    if (index < countDir() ? QDir(absoluteFilePath(page[index])).removeRecursively() : QDir::remove(page[index])) {
        refresh();
        return true;
    }
    return false;
}

void Directory::nextPage() {
#ifdef Q_OS_WIN
    if (isThisPC) return;
#endif
    if (++pageIndex * pageSize >= (int)QDir::count()) --pageIndex;
    page = fullEntryList.mid(pageIndex * pageSize, pageSize);
    //page = QDir::entryList().mid(pageIndex * pageSize, pageSize);
}

void Directory::prevPage() {
#ifdef Q_OS_WIN
    if (isThisPC) return;
#endif
    if (pageIndex > 0) --pageIndex;
    page = fullEntryList.mid(pageIndex * pageSize, pageSize);
    //page = QDir::entryList().mid(pageIndex * pageSize, pageSize);
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

void Directory::update() {
    pageIndex = 0;
#ifdef Q_OS_WIN
    if (isThisPC) {
        page.clear();
        for (auto drive : QDir::drives()) page << drive.filePath();
        /* warning: assume drives are less than slots */
        imageList.clear();
        imageIndex = 0;
        return;
    }
#endif

    fullEntryList.clear();
    fullEntryList << QDir::entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::IgnoreCase);
    for (const QStringList &filter : typeFilters)
        fullEntryList << QDir::entryList(filter, QDir::Files, QDir::IgnoreCase);
    fullEntryList << QDir::entryList(QDir::Files, QDir::IgnoreCase);
    fullEntryList.removeDuplicates();

    page = fullEntryList.mid(pageIndex * pageSize, pageSize);

    imageList = QDir::entryList(typeFilters[0], QDir::Files);
    imageIndex = 0;
}
