#ifndef SPELL_CHECKER_HIGHLIGHTER_H
#define SPELL_CHECKER_HIGHLIGHTER_H

#include "spell_checker.h"

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class SpellCheckerHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    SpellCheckerHighlighter(QTextDocument* parent, const SpellChecker* checker);

protected:
    void highlightBlock(const QString& text) override;

private:
    const SpellChecker* checker;
    QTextCharFormat misspelledFormat;
};

#endif
