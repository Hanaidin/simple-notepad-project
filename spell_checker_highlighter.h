#ifndef SPELL_CHECKER_HIGHLIGHTER_H
#define SPELL_CHECKER_HIGHLIGHTER_H

#include "spell_checker.h"

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class SpellCheckerHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    enum class SyntaxMode {
        PlainText,
        Cpp,
        Python,
    };

    SpellCheckerHighlighter(QTextDocument* parent, const SpellChecker* checker);
    void set_syntax_mode(SyntaxMode mode);

protected:
    void highlightBlock(const QString& text) override;

private:
    void highlight_keywords(const QString& text);
    void highlight_spelling(const QString& text);

    const SpellChecker* checker;
    SyntaxMode syntaxMode;
    QTextCharFormat misspelledFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat commentFormat;
};

#endif
