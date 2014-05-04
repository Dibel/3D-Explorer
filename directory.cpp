#include "directory.h"

Directory::Directory() : QDir() {
#ifdef Q_OS_WIN
    isThisPc = false;
#endif
    setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    setSorting(QDir::DirsFirst | QDir::IgnoreCase);
    updateLists();
}

#ifdef Q_OS_WIN
QString Directory::absolutePath() {
    return isThisPc ? "This PC" : QDir::absolutePath();
}

bool Directory::cd(const QString &dirName) {
    if (isThisPc) {
        setPath(dirName);
        if (exists()) isThisPc = false;
        return !isThisPc;
    }
    return QDir::cd(dirName);
}

bool cdUp() {
    if (isThisPc) return false;
    if (isRoot) { isThisPc = true; return true; }
    return QDir::cdUp();
}

QStringList Directory::entryList() {
    if (isThisPc) {
        QStringList ret;
        for (auto drive : QDir::drives())
            ret << drive.filePath();
        return ret;
    }
    return QDir::entryList();
}
#endif

int Directory::countDir() {
    return dirCnt;
}

QString Directory::getImage() {
    return imageList.isEmpty() ? ":/model/photo.png"
        : absoluteFilePath(imageList[curImageIdx]);
}

QString Directory::getNextImage() {
    if (imageList.isEmpty()) return ":/model/photo.png";
    if (++curImageIdx >= imageList.size())
        curImageIdx -= imageList.size();
    return absoluteFilePath(imageList[curImageIdx]);
}

void Directory::updateLists() {
    dirCnt = QDir::entryList(QDir::Dirs | QDir::NoDot).size();

    static const QStringList imageFilter = {
        "*.bmp", "*.jpg", "*.jpeg", "*.gif", "*.png" };
    imageList = QDir::entryList(imageFilter, QDir::Files);
    curImageIdx = 0;
}
