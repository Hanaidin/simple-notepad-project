#include "main_window.h"

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QTextEdit>

#include <cassert>

bool has_action(MainWindow& window, const QString& text)
{
    for (QAction* action : window.findChildren<QAction*>()) {
        QString actionText = action->text();
        actionText.remove('&');
        if (actionText == text)
            return true;
    }

    return false;
}

QAction* find_action(MainWindow& window, const QString& text)
{
    for (QAction* action : window.findChildren<QAction*>()) {
        QString actionText = action->text();
        actionText.remove('&');
        if (actionText == text)
            return action;
    }

    return nullptr;
}

bool has_label_text(MainWindow& window, const QString& text)
{
    for (QLabel* label : window.findChildren<QLabel*>()) {
        if (label->text().contains(text))
            return true;
    }

    return false;
}

bool has_menu(MainWindow& window, const QString& title)
{
    for (QMenu* menu : window.findChildren<QMenu*>()) {
        QString menuTitle = menu->title();
        menuTitle.remove('&');
        if (menuTitle == title)
            return true;
    }

    return false;
}

bool wait_for_console_text(QPlainTextEdit* console, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 5000) {
        QApplication::processEvents();
        if (console->toPlainText().contains(text))
            return true;
    }

    return false;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    assert(window.windowTitle() == "Notepad");

    assert(has_menu(window, "File"));
    assert(has_menu(window, "Edit"));
    assert(has_menu(window, "Format"));
    assert(has_menu(window, "Tools"));
    assert(has_menu(window, "View"));

    assert(has_action(window, "Check Spelling..."));
    assert(has_action(window, "Word Frequency"));
    assert(has_action(window, "Export PDF..."));
    assert(has_action(window, "Dark Theme"));
    assert(has_action(window, "Restore Autosaved Draft"));
    assert(has_action(window, "Run Python"));
    assert(has_action(window, "Stop Python"));
    assert(has_action(window, "Clear Python Console"));
    assert(has_action(window, "Show Python Console"));
    assert(has_action(window, "C++"));
    assert(has_action(window, "Python"));

    QTextEdit* editor = window.findChild<QTextEdit*>();
    assert(editor != nullptr);
    editor->setPlainText("hello world");

    QAction* zoomIn = find_action(window, "Zoom In");
    QAction* zoomOut = find_action(window, "Zoom Out");
    QAction* resetZoom = find_action(window, "Reset Zoom");
    assert(zoomIn != nullptr);
    assert(zoomOut != nullptr);
    assert(resetZoom != nullptr);

    zoomIn->trigger();
    assert(has_label_text(window, "Zoom: 110%"));

    zoomOut->trigger();
    assert(has_label_text(window, "Zoom: 100%"));

    zoomOut->trigger();
    assert(has_label_text(window, "Zoom: 90%"));

    resetZoom->trigger();
    assert(has_label_text(window, "Zoom: 100%"));

    QAction* uppercase = find_action(window, "UPPERCASE");
    QAction* lowercase = find_action(window, "lowercase");
    assert(uppercase != nullptr);
    assert(lowercase != nullptr);

    editor->setPlainText("hello world");
    uppercase->trigger();
    assert(editor->toPlainText() == "HELLO WORLD");
    lowercase->trigger();
    assert(editor->toPlainText() == "hello world");

    QAction* bold = find_action(window, "Bold");
    QAction* italic = find_action(window, "Italic");
    QAction* underline = find_action(window, "Underline");
    assert(bold != nullptr);
    assert(italic != nullptr);
    assert(underline != nullptr);
    bold->trigger();
    italic->trigger();
    underline->trigger();
    assert(bold->isChecked());
    assert(italic->isChecked());
    assert(underline->isChecked());

    QAction* darkTheme = find_action(window, "Dark Theme");
    assert(darkTheme != nullptr);
    darkTheme->trigger();
    assert(darkTheme->isChecked());
    darkTheme->trigger();
    assert(!darkTheme->isChecked());

    QPlainTextEdit* console = window.findChild<QPlainTextEdit*>("pythonConsole");
    assert(console != nullptr);
    QAction* runPython = find_action(window, "Run Python");
    QAction* clearPython = find_action(window, "Clear Python Console");
    assert(runPython != nullptr);
    assert(clearPython != nullptr);
    editor->setPlainText("print(40 + 2)");
    runPython->trigger();
    assert(wait_for_console_text(console, "42"));
    clearPython->trigger();
    assert(console->toPlainText().isEmpty());

    return 0;
}
