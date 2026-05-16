#ifndef AUTOSAVE_MANAGER_H
#define AUTOSAVE_MANAGER_H

#include <QString>

class AutosaveManager {
public:
    explicit AutosaveManager(const QString& path = default_path());

    static QString default_path();

    bool has_draft() const;
    bool write_html(const QString& html) const;
    QString read_html(bool* ok = nullptr) const;
    void clear() const;

private:
    QString autosavePath;
};

#endif
