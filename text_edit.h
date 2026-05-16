#ifndef TEXT_EDIT_H
#define TEXT_EDIT_H

#include <QTextEdit>

class LineNumberArea;

class TextEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit TextEdit(QWidget* parent = nullptr);

    int line_number_area_width() const;
    void line_number_area_paint_event(QPaintEvent* event);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void update_line_number_area_width();
    void update_line_number_area();

private:
    LineNumberArea* lineNumberArea;
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(TextEdit* editor)
        : QWidget(editor)
        , editor(editor)
    {
    }

    QSize sizeHint() const override { return QSize(editor->line_number_area_width(), 0); }

protected:
    void paintEvent(QPaintEvent* event) override { editor->line_number_area_paint_event(event); }

private:
    TextEdit* editor;
};

#endif
