#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "find_replace_dialog.h"
#include "spell_checker.h"
#include "spell_checker_highlighter.h"
#include "text_edit.h"
#include "word_frequency_dialog.h"

#include <QAction>
#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPointer>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void new_file();
    void open_file();
    void save_file();
    void save_file_as();
    void print_document();
    void update_title();

    void show_find_replace_dialog();
    void find_next();
    void replace_current();
    void replace_all();
    void show_word_frequency();

    void apply_bold();
    void apply_italic();
    void apply_underline();
    void choose_font();
    void choose_text_color();

    void transform_uppercase();
    void transform_lowercase();
    void transform_capitalize();
    void transform_sentence_case();
    void transform_swap_case();

    void zoom_in();
    void zoom_out();
    void reset_zoom();
    void check_spelling();
    void show_editor_context_menu(const QPoint& position);
    void update_status_bar();
    void open_recent_file();

private:
    void create_actions();
    void create_menus();
    void create_toolbar();
    void create_status_bar();
    void connect_editor_signals();
    void load_spell_checker();

    void load_file(const QString& path);
    void write_file(const QString& path);
    bool ask_to_save_if_modified();
    void show_error(const QString& message);
    void add_recent_file(const QString& path);
    void update_recent_files_menu();

    void merge_format_on_selection(const QTextCharFormat& format);
    void apply_transform(QString (*transform)(QString));
    std::vector<std::pair<QString, int>> collect_word_frequency() const;
    QString preserve_case(const QString& original, const QString& suggestion) const;

    TextEdit* editor;
    SpellChecker spellChecker;
    SpellCheckerHighlighter* spellHighlighter;
    QPointer<FindReplaceDialog> findReplaceDialog;

    QString currentFilePath;
    int zoomSteps;

    QMenu* fileMenu;
    QMenu* editMenu;
    QMenu* formatMenu;
    QMenu* toolsMenu;
    QMenu* viewMenu;
    QMenu* recentFilesMenu;

    QAction* saveAction;
    QAction* boldAction;
    QAction* italicAction;
    QAction* underlineAction;
    QAction* recentFileActions[5];

    QLabel* wordLineLabel;
    QLabel* cursorLabel;
    QLabel* zoomLabel;
};

#endif
