#include "spell_checker_highlighter.h"

#include <QRegularExpression>

SpellCheckerHighlighter::SpellCheckerHighlighter(QTextDocument* parent, const SpellChecker* checker)
    : QSyntaxHighlighter(parent)
    , checker(checker)
{
    misspelledFormat.setUnderlineColor(Qt::red);
    misspelledFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}

void SpellCheckerHighlighter::highlightBlock(const QString& text)
{
    if (checker == nullptr || !checker->is_loaded())
        return;

    static const QRegularExpression wordExpression("[A-Za-z]+(?:'[A-Za-z]+)?");
    QRegularExpressionMatchIterator iterator = wordExpression.globalMatch(text);

    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString word = match.captured();

        if (!checker->is_correct(word.toStdString()))
            setFormat(match.capturedStart(), match.capturedLength(), misspelledFormat);
    }
}
