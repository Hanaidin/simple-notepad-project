#include "autosave_manager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

AutosaveManager::AutosaveManager(const QString& path)
    : autosavePath(path)
{
}

QString AutosaveManager::default_path()
{
    QString directoryPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(directoryPath);
    return directoryPath + "/autosave.html";
}

bool AutosaveManager::has_draft() const
{
    QFile file(autosavePath);
    return file.exists() && file.size() > 0;
}

bool AutosaveManager::write_html(const QString& html) const
{
    QFile file(autosavePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    stream << html;
    return true;
}

QString AutosaveManager::read_html(bool* ok) const
{
    QFile file(autosavePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (ok != nullptr)
            *ok = false;
        return {};
    }

    QTextStream stream(&file);
    if (ok != nullptr)
        *ok = true;
    return stream.readAll();
}

void AutosaveManager::clear() const { QFile::remove(autosavePath); }
