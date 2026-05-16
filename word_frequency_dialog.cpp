#include "word_frequency_dialog.h"

#include "ui_word_frequency_dialog.h"

#include <QHeaderView>
#include <QTableWidgetItem>

WordFrequencyDialog::WordFrequencyDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WordFrequencyDialog)
{
    ui->setupUi(this);
    setWindowTitle("Word Frequency");
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels({ "Word", "Count" });
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

WordFrequencyDialog::~WordFrequencyDialog() { delete ui; }

void WordFrequencyDialog::set_words(const std::vector<std::pair<QString, int>>& words)
{
    ui->tableWidget->setRowCount(static_cast<int>(words.size()));

    for (int row = 0; row < static_cast<int>(words.size()); ++row) {
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(words[row].first));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(words[row].second)));
    }

    ui->tableWidget->resizeColumnsToContents();
}
