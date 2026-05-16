#include "spell_checker_highlighter.h"

#include <QFont>
#include <QRegularExpression>

SpellCheckerHighlighter::SpellCheckerHighlighter(QTextDocument* parent, const SpellChecker* checker)
    : QSyntaxHighlighter(parent)
    , checker(checker)
    , syntaxMode(SyntaxMode::PlainText)
{
    misspelledFormat.setUnderlineColor(Qt::red);
    misspelledFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    keywordFormat.setForeground(QColor(86, 61, 124));
    keywordFormat.setFontWeight(QFont::Bold);

    commentFormat.setForeground(QColor(91, 130, 93));
    commentFormat.setFontItalic(true);
}

void SpellCheckerHighlighter::set_syntax_mode(SyntaxMode mode)
{
    syntaxMode = mode;
    rehighlight();
}

void SpellCheckerHighlighter::highlightBlock(const QString& text)
{
    highlight_keywords(text);
    highlight_spelling(text);
}

void SpellCheckerHighlighter::highlight_keywords(const QString& text)
{
    if (syntaxMode == SyntaxMode::PlainText)
        return;

    QStringList keywords;
    QString commentPattern;

    if (syntaxMode == SyntaxMode::Cpp) {
        keywords = {
            "auto",
            "bool",
            "break",
            "case",
            "catch",
            "char",
            "class",
            "const",
            "continue",
            "delete",
            "double",
            "else",
            "enum",
            "explicit",
            "false",
            "float",
            "for",
            "if",
            "include",
            "int",
            "namespace",
            "new",
            "nullptr",
            "private",
            "protected",
            "public",
            "return",
            "signals",
            "slots",
            "static",
            "struct",
            "switch",
            "this",
            "throw",
            "true",
            "try",
            "using",
            "void",
            "while",
        };
        commentPattern = "//[^\\n]*";
    } else {
        keywords = {
            "and",
            "as",
            "assert",
            "break",
            "class",
            "continue",
            "def",
            "elif",
            "else",
            "except",
            "False",
            "finally",
            "for",
            "from",
            "if",
            "import",
            "in",
            "is",
            "lambda",
            "None",
            "not",
            "or",
            "pass",
            "raise",
            "return",
            "True",
            "try",
            "while",
            "with",
            "yield",
        };
        commentPattern = "#[^\\n]*";
    }

    for (const QString& keyword : keywords) {
        QRegularExpression expression(QString("\\b%1\\b").arg(QRegularExpression::escape(keyword)));
        QRegularExpressionMatchIterator iterator = expression.globalMatch(text);
        while (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), keywordFormat);
        }
    }

    QRegularExpression commentExpression(commentPattern);
    QRegularExpressionMatchIterator comments = commentExpression.globalMatch(text);
    while (comments.hasNext()) {
        QRegularExpressionMatch match = comments.next();
        setFormat(match.capturedStart(), match.capturedLength(), commentFormat);
    }
}

void SpellCheckerHighlighter::highlight_spelling(const QString& text)
{
    if (checker == nullptr || !checker->is_loaded())
        return;

    static const QRegularExpression wordExpression("[A-Za-z]+(?:'[A-Za-z]+)?");
    QRegularExpressionMatchIterator iterator = wordExpression.globalMatch(text);

    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString word = match.captured();

        if (!checker->is_correct(word.toStdString())) {
            QTextCharFormat combinedFormat = format(match.capturedStart());
            combinedFormat.setUnderlineColor(misspelledFormat.underlineColor());
            combinedFormat.setUnderlineStyle(misspelledFormat.underlineStyle());
            setFormat(match.capturedStart(), match.capturedLength(), combinedFormat);
        }
    }
}
