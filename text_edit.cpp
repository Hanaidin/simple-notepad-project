#include "text_edit.h"

#include <QAbstractTextDocumentLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>

TextEdit::TextEdit(QWidget* parent)
    : QTextEdit(parent)
    , lineNumberArea(new LineNumberArea(this))
{
    setAcceptRichText(true);
    setLineWrapMode(QTextEdit::WidgetWidth);
    setViewportMargins(line_number_area_width(), 0, 0, 0);

    connect(document(), &QTextDocument::blockCountChanged, this,
        &TextEdit::update_line_number_area_width);
    connect(
        verticalScrollBar(), &QScrollBar::valueChanged, this, &TextEdit::update_line_number_area);
    connect(this, &QTextEdit::textChanged, this, &TextEdit::update_line_number_area);
    connect(this, &QTextEdit::cursorPositionChanged, this, &TextEdit::update_line_number_area);
}

int TextEdit::line_number_area_width() const
{
    int digits = 1;
    int maximum = qMax(1, document()->blockCount());

    while (maximum >= 10) {
        maximum /= 10;
        ++digits;
    }

    return 14 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void TextEdit::line_number_area_paint_event(QPaintEvent* event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(245, 247, 250));
    painter.setPen(QColor(108, 117, 125));

    QTextBlock block = document()->begin();
    int blockNumber = 1;

    while (block.isValid()) {
        QTextCursor cursor(block);
        QRect rectangle = cursorRect(cursor);

        if (rectangle.bottom() >= event->rect().top()
            && rectangle.top() <= event->rect().bottom()) {
            painter.drawText(0, rectangle.top(), lineNumberArea->width() - 6,
                fontMetrics().height(), Qt::AlignRight, QString::number(blockNumber));
        }

        if (rectangle.top() > event->rect().bottom())
            break;

        block = block.next();
        ++blockNumber;
    }
}

void TextEdit::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);

    QRect contents = contentsRect();
    lineNumberArea->setGeometry(
        QRect(contents.left(), contents.top(), line_number_area_width(), contents.height()));
}

void TextEdit::update_line_number_area_width()
{
    setViewportMargins(line_number_area_width(), 0, 0, 0);
    update_line_number_area();
}

void TextEdit::update_line_number_area() { lineNumberArea->update(); }
