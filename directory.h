#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtCore/QDir>
#include <QtGui/QImage>

/**
 * \brief A cross-platform wrapper for directories and files
 *
 * Load the name and type of entries in a directory and group
 * them into "page" with a fit size for containers in a room.
 *
 * The "type" of entries is currently determined by extension name.
 *
 * In Microsoft Windows, this class treats "This PC"
 * ("My Computer") as a directory and all drivers as its
 * subdirectory.
 *
 * The behaviour on Apple Mac OS has not been verified yet,
 * due to the limitation of devices. The class is expected to
 * work fine as Mac OS is a UNIX-like system.
 */

class Directory : public QDir {
public:
    /// Create a wrapper for current working directory.
    /// The page size is set to @p size.
    Directory(int size);

    /// Change working directory to subdirectory at @p index in current page.
    /// Return true if success, false otherwise.
    bool cd(int index);

    /// Change working directory to the parent of current one.
    /// List all drivers if current directory is the root of a driver.
    /// Return true if success, false otherwise.
    bool cdUp();

    /// Move to next page. Do nothing when reach the end.
    void nextPage();
    /// Move to previous page. Do nothing when reach the beginning.
    void prevPage();

    /// Reload current page.
    /// Use this function when the content is modified by other programs.
    void refresh();

    /// Show a confirm dialog and remove the entry at @p index in current page
    /// if user pressed "yes". If the entry is a directory, remove it recursively.
    /// The directory will be reloaded after removing.
    /// Return true if entry removed, false otherwise.
    /// @warning The entry is not moved to recycler but permanently deleted.
    bool remove(int index);

    /// Return the count of all entries in current page, including subdirectories.
    inline int count() const
    {
        return qMin(entryList.size() - offset, pageSize);
    }

    /// Return the count of subdirectories in current page.
    inline int countDir() const
    {
        return qMax(dirCnt - offset, 0);
    }

    /// Return the file name (or directory name).
    inline QString entry(int index) const
    {
        return entryList.at(offset + index);
    }

    /// Return the index in @c fileTypeList plus 2 if the entry name
    /// matches any filter, otherwise 0 for directories and 1 for files.
    inline int entryType(int index) const
    {
        return typeList.at(offset + index);
    }

    /// Return type id for all entries.
    inline QVector<int> entryTypeList() const
    {
        return typeList.mid(offset, pageSize);
    }

    /// Return the absolute path of a file in directory.
    /// Does not check if the file actually exists.
    inline QString absoluteFilePath(const QString &fileName) const
    {
        return QDir::absoluteFilePath(fileName);
    }

    /// Return the absolute path of file at @index in current page.
    inline QString absoluteFilePath(int index) const
    {
        return absoluteFilePath(entry(index));
    }

    /// Mark a file as being previewed and return its absolute path
    /// if it is of @p assumedType, return an empty string if not.
    /// Exactly one file of each type can be previewed at same time.
    QString playFile(int index, const QString &assumedType);

    /// Mark next file of @p type in current page as being previewed
    /// and return the absolute path. If the current previewing file is the
    /// last one in the page, roll back to first one. If no file matches the
    /// type, return an empty string.
    QString playNext(const QString &type);

    /// Mark previous file of @p type in current page as being previewed
    /// and return the absolute path. If the current previewing file is the
    /// first one in the page, roll back to last one. If no file matches the
    /// type, return an empty string.
    QString playPrev(const QString &type);

    /// Return the absolute path of previewing file of @p type
    QString getPlayingFile(const QString &type);

#ifdef Q_OS_WIN
    /// Return "This PC" if Directory is listing all drivers, otherwise
    /// the absolute path of current directory.
    QString absolutePath() const;
#endif

private:
    void update();

#ifdef Q_OS_WIN
    bool isThisPC;
#endif

    int pageSize = -1;
    int offset = 0; // start index of current page
    int dirCnt = 0;

    QStringList entryList;
    QVector<int> typeList;

    QVector<int> playingFiles;
};

#endif
