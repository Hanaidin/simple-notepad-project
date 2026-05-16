#ifndef FIND_REPLACE_DIALOG_H
#define FIND_REPLACE_DIALOG_H

#include <QDialog>

namespace Ui {
class FindReplaceDialog;
}

class FindReplaceDialog : public QDialog {
    Q_OBJECT

public:
    explicit FindReplaceDialog(QWidget* parent = nullptr);
    ~FindReplaceDialog() override;

    QString find_text() const;
    QString replace_text() const;

signals:
    void find_next_requested();
    void replace_requested();
    void replace_all_requested();

private:
    Ui::FindReplaceDialog* ui;
};

#endif
