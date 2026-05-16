#include "main_window.h"

#include <QAction>
#include <QApplication>
#include <QMenu>

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
    assert(has_action(window, "C++"));
    assert(has_action(window, "Python"));

    return 0;
}
