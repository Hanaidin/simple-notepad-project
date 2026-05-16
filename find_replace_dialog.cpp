#include "find_replace_dialog.h"

#include "ui_find_replace_dialog.h"

FindReplaceDialog::FindReplaceDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::FindReplaceDialog)
{
    ui->setupUi(this);
    setWindowTitle("Find / Replace");

    connect(
        ui->findNextButton, &QPushButton::clicked, this, &FindReplaceDialog::find_next_requested);
    connect(ui->replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::replace_requested);
    connect(ui->replaceAllButton, &QPushButton::clicked, this,
        &FindReplaceDialog::replace_all_requested);
    connect(ui->closeButton, &QPushButton::clicked, this, &FindReplaceDialog::close);
}

FindReplaceDialog::~FindReplaceDialog() { delete ui; }

QString FindReplaceDialog::find_text() const { return ui->findLineEdit->text(); }

QString FindReplaceDialog::replace_text() const { return ui->replaceLineEdit->text(); }
