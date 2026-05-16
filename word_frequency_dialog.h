#ifndef WORD_FREQUENCY_DIALOG_H
#define WORD_FREQUENCY_DIALOG_H

#include <QDialog>

#include <utility>
#include <vector>

namespace Ui {
class WordFrequencyDialog;
}

class WordFrequencyDialog : public QDialog {
    Q_OBJECT

public:
    explicit WordFrequencyDialog(QWidget* parent = nullptr);
    ~WordFrequencyDialog() override;

    void set_words(const std::vector<std::pair<QString, int>>& words);

private:
    Ui::WordFrequencyDialog* ui;
};

#endif
