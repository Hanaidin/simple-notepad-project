#include "main_window.h"

#include "notepad_exception.h"
#include "sort.h"
#include "text_transform.h"

#include <QActionGroup>
#include <QApplication>
#include <QColorDialog>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QIcon>
#include <QKeySequence>
#include <QMap>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QRegularExpression>
#include <QSettings>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <memory>
#include <numeric>

namespace {
const int maximumRecentFiles = 5;

QString recent_files_key() { return "recentFiles"; }

QString icon_path(const QString& fileName) { return QString("data/images/%1").arg(fileName); }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , editor(new TextEdit(this))
    , spellHighlighter(nullptr)
    , zoomSteps(0)
    , cachedWordCount(0)
    , fileMenu(nullptr)
    , editMenu(nullptr)
    , formatMenu(nullptr)
    , toolsMenu(nullptr)
    , viewMenu(nullptr)
    , recentFilesMenu(nullptr)
    , saveAction(nullptr)
    , boldAction(nullptr)
    , italicAction(nullptr)
    , underlineAction(nullptr)
    , darkThemeAction(nullptr)
    , restoreAutosaveAction(nullptr)
    , runPythonAction(nullptr)
    , stopPythonAction(nullptr)
    , clearPythonConsoleAction(nullptr)
    , showPythonConsoleAction(nullptr)
    , wordLineLabel(nullptr)
    , cursorLabel(nullptr)
    , zoomLabel(nullptr)
    , autosaveLabel(nullptr)
    , autosaveTimer(new QTimer(this))
    , pythonConsoleDock(nullptr)
    , pythonConsole(nullptr)
    , pythonProcess(new QProcess(this))
{
    setCentralWidget(editor);
    setWindowTitle("Notepad");
    resize(980, 680);
    setUnifiedTitleAndToolBarOnMac(true);

    load_spell_checker();
    spellHighlighter = new SpellCheckerHighlighter(editor->document(), &spellChecker);

    create_actions();
    create_menus();
    create_toolbar();
    create_status_bar();
    create_python_console();
    connect_editor_signals();
    setup_autosave();

    QSettings settings;
    apply_theme(settings.value("darkTheme", false).toBool());
    try_restore_autosave();

    update_recent_files_menu();
    update_status_bar();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (ask_to_save_if_modified())
        event->accept();
    else
        event->ignore();
}

void MainWindow::new_file()
{
    if (!ask_to_save_if_modified())
        return;

    editor->clear();
    currentFilePath.clear();
    editor->document()->setModified(false);
    clear_autosave();
    update_title();
}

void MainWindow::open_file()
{
    if (!ask_to_save_if_modified())
        return;

    QString path = QFileDialog::getOpenFileName(this, "Open File", QString(),
        "Text files (*.txt *.md *.cpp *.h);;HTML files (*.html *.htm);;All files (*)");

    if (path.isEmpty())
        return;

    try {
        load_file(path);
    } catch (const notepad_exception& exception) {
        show_error(exception.what());
    }
}

void MainWindow::save_file()
{
    if (currentFilePath.isEmpty()) {
        save_file_as();
        return;
    }

    try {
        write_file(currentFilePath);
    } catch (const notepad_exception& exception) {
        show_error(exception.what());
    }
}

void MainWindow::save_file_as()
{
    QString path = QFileDialog::getSaveFileName(this, "Save File", currentFilePath,
        "Text files (*.txt);;Markdown (*.md);;HTML files (*.html);;All files (*)");

    if (path.isEmpty())
        return;

    try {
        write_file(path);
        currentFilePath = path;
        add_recent_file(path);
        update_title();
    } catch (const notepad_exception& exception) {
        show_error(exception.what());
    }
}

void MainWindow::print_document()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);

    if (dialog.exec() == QDialog::Accepted)
        editor->print(&printer);
}

void MainWindow::export_pdf()
{
    QString path = QFileDialog::getSaveFileName(this, "Export PDF", QString(), "PDF files (*.pdf)");

    if (path.isEmpty())
        return;

    if (!path.endsWith(".pdf", Qt::CaseInsensitive))
        path += ".pdf";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    editor->document()->print(&printer);
    statusBar()->showMessage("PDF exported.", 2500);
}

void MainWindow::update_title()
{
    if (currentFilePath.isEmpty()) {
        setWindowTitle("Notepad");
    } else {
        setWindowTitle(QString("Notepad: %1").arg(currentFilePath));
    }
}

void MainWindow::restore_autosave_draft()
{
    if (!ask_to_save_if_modified())
        return;

    bool ok = false;
    QString html = autosaveManager.read_html(&ok);
    if (!ok) {
        autosaveLabel->setText("Autosave: unavailable");
        restoreAutosaveAction->setEnabled(false);
        return;
    }

    editor->setHtml(html);
    currentFilePath.clear();
    editor->document()->setModified(true);
    restoreAutosaveAction->setEnabled(false);
    autosaveLabel->setText("Autosave: restored");
    update_title();
}

void MainWindow::show_find_replace_dialog()
{
    if (findReplaceDialog == nullptr) {
        findReplaceDialog = new FindReplaceDialog(this);
        connect(findReplaceDialog, &FindReplaceDialog::find_next_requested, this,
            &MainWindow::find_next);
        connect(findReplaceDialog, &FindReplaceDialog::replace_requested, this,
            &MainWindow::replace_current);
        connect(findReplaceDialog, &FindReplaceDialog::replace_all_requested, this,
            &MainWindow::replace_all);
    }

    findReplaceDialog->show();
    findReplaceDialog->raise();
    findReplaceDialog->activateWindow();
}

void MainWindow::find_next()
{
    if (findReplaceDialog == nullptr || findReplaceDialog->find_text().isEmpty())
        return;

    if (editor->find(findReplaceDialog->find_text()))
        return;

    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(cursor);

    if (!editor->find(findReplaceDialog->find_text()))
        QMessageBox::information(this, "Find / Replace", "Text was not found.");
}

void MainWindow::replace_current()
{
    if (findReplaceDialog == nullptr)
        return;

    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == findReplaceDialog->find_text())
        cursor.insertText(findReplaceDialog->replace_text());

    find_next();
}

void MainWindow::replace_all()
{
    if (findReplaceDialog == nullptr || findReplaceDialog->find_text().isEmpty())
        return;

    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(cursor);

    int count = 0;
    while (editor->find(findReplaceDialog->find_text())) {
        QTextCursor match = editor->textCursor();
        match.insertText(findReplaceDialog->replace_text());
        ++count;
    }

    cursor.endEditBlock();
    QMessageBox::information(
        this, "Find / Replace", QString("Replaced %1 occurrence(s).").arg(count));
}

void MainWindow::show_word_frequency()
{
    WordFrequencyDialog dialog(this);
    dialog.set_words(collect_word_frequency());
    dialog.exec();
}

void MainWindow::apply_bold()
{
    QTextCharFormat format;
    format.setFontWeight(boldAction->isChecked() ? QFont::Bold : QFont::Normal);
    merge_format_on_selection(format);
}

void MainWindow::apply_italic()
{
    QTextCharFormat format;
    format.setFontItalic(italicAction->isChecked());
    merge_format_on_selection(format);
}

void MainWindow::apply_underline()
{
    QTextCharFormat format;
    format.setFontUnderline(underlineAction->isChecked());
    merge_format_on_selection(format);
}

void MainWindow::choose_font()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, editor->currentFont(), this, "Choose Font");

    if (!ok)
        return;

    QTextCharFormat format;
    format.setFont(font);
    merge_format_on_selection(format);
}

void MainWindow::choose_text_color()
{
    QColor color = QColorDialog::getColor(editor->textColor(), this, "Choose Text Color");

    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setForeground(color);
    merge_format_on_selection(format);
}

void MainWindow::transform_uppercase() { apply_transform(text_transform::uppercase); }

void MainWindow::transform_lowercase() { apply_transform(text_transform::lowercase); }

void MainWindow::transform_capitalize() { apply_transform(text_transform::capitalize); }

void MainWindow::transform_sentence_case() { apply_transform(text_transform::sentence_case); }

void MainWindow::transform_swap_case() { apply_transform(text_transform::swap_case); }

void MainWindow::zoom_in()
{
    editor->zoomIn(1);
    ++zoomSteps;
    update_status_bar();
}

void MainWindow::zoom_out()
{
    editor->zoomOut(1);
    --zoomSteps;
    update_status_bar();
}

void MainWindow::reset_zoom()
{
    if (zoomSteps > 0)
        editor->zoomOut(zoomSteps);
    else if (zoomSteps < 0)
        editor->zoomIn(-zoomSteps);

    zoomSteps = 0;
    update_status_bar();
}

void MainWindow::toggle_dark_theme()
{
    bool darkMode = darkThemeAction->isChecked();
    QSettings settings;
    settings.setValue("darkTheme", darkMode);
    apply_theme(darkMode);
}

void MainWindow::autosave_draft()
{
    if (!editor->document()->isModified()) {
        autosaveLabel->setText("Autosave: idle");
        return;
    }

    if (!autosaveManager.write_html(editor->toHtml())) {
        autosaveLabel->setText("Autosave: failed");
        return;
    }

    autosaveLabel->setText("Autosave: saved");
}

void MainWindow::run_python_code()
{
    if (pythonProcess->state() != QProcess::NotRunning) {
        pythonConsoleDock->show();
        pythonConsole->appendPlainText("Python is already running.");
        return;
    }

    QTextCursor cursor = editor->textCursor();
    QString code = cursor.hasSelection() ? cursor.selectedText() : editor->toPlainText();
    code.replace(QChar::ParagraphSeparator, "\n");

    if (code.trimmed().isEmpty()) {
        pythonConsoleDock->show();
        pythonConsole->appendPlainText("Nothing to run.");
        return;
    }

    pythonConsoleDock->show();
    pythonConsole->appendPlainText(
        QString("\n>>> Run Python %1").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
    pythonConsole->appendPlainText(code);
    pythonConsole->appendPlainText("--- output ---");

    runPythonAction->setEnabled(false);
    stopPythonAction->setEnabled(true);

    pythonProcess->start("python3", { "-u", "-" });
    if (!pythonProcess->waitForStarted(3000)) {
        pythonConsole->appendPlainText("Could not start python3.");
        runPythonAction->setEnabled(true);
        stopPythonAction->setEnabled(false);
        return;
    }

    pythonProcess->write(code.toUtf8());
    pythonProcess->closeWriteChannel();
}

void MainWindow::stop_python_code()
{
    if (pythonProcess->state() == QProcess::NotRunning)
        return;

    pythonConsoleDock->show();
    pythonConsole->appendPlainText("Stopping Python process...");
    pythonProcess->kill();
}

void MainWindow::clear_python_console() { pythonConsole->clear(); }

void MainWindow::handle_python_stdout()
{
    pythonConsole->appendPlainText(
        QString::fromUtf8(pythonProcess->readAllStandardOutput()).trimmed());
}

void MainWindow::handle_python_stderr()
{
    pythonConsole->appendPlainText(
        QString::fromUtf8(pythonProcess->readAllStandardError()).trimmed());
}

void MainWindow::handle_python_finished(int exitCode, QProcess::ExitStatus status)
{
    QString statusText = status == QProcess::NormalExit ? "finished" : "stopped";
    pythonConsole->appendPlainText(
        QString("--- Python %1, exit code %2 ---").arg(statusText).arg(exitCode));
    runPythonAction->setEnabled(true);
    stopPythonAction->setEnabled(false);
}

void MainWindow::handle_python_error(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    pythonConsoleDock->show();
    pythonConsole->appendPlainText("Python runner error. Make sure python3 is installed.");
    runPythonAction->setEnabled(true);
    stopPythonAction->setEnabled(false);
}

void MainWindow::check_spelling()
{
    spellHighlighter->rehighlight();
    statusBar()->showMessage("Spell check completed.", 2500);
}

void MainWindow::show_editor_context_menu(const QPoint& position)
{
    QTextCursor cursor = editor->cursorForPosition(position);
    cursor.select(QTextCursor::WordUnderCursor);
    QString selectedWord = cursor.selectedText();

    QMenu menu(this);
    std::unique_ptr<QMenu> standardMenu(editor->createStandardContextMenu(position));

    if (!selectedWord.isEmpty() && !spellChecker.is_correct(selectedWord.toStdString())) {
        std::vector<std::string> suggestions = spellChecker.suggestions(selectedWord.toStdString());

        if (suggestions.empty()) {
            QAction* action = menu.addAction("No spelling suggestions");
            action->setEnabled(false);
        } else {
            for (const std::string& suggestion : suggestions) {
                QString replacement
                    = preserve_case(selectedWord, QString::fromStdString(suggestion));
                menu.addAction(replacement, this, [this, cursor, replacement]() mutable {
                    QTextCursor editableCursor = cursor;
                    editableCursor.insertText(replacement);
                    editor->setTextCursor(editableCursor);
                });
            }
        }

        menu.addSeparator();
    }

    for (QAction* action : standardMenu->actions())
        menu.addAction(action);

    menu.exec(editor->mapToGlobal(position));
}

void MainWindow::update_status_bar()
{
    int lines = qMax(1, editor->document()->blockCount());

    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.positionInBlock() + 1;

    wordLineLabel->setText(QString("Words: %1   Lines: %2").arg(cachedWordCount).arg(lines));
    cursorLabel->setText(QString("Ln %1, Col %2").arg(line).arg(column));
    zoomLabel->setText(QString("Zoom: %1%").arg(100 + zoomSteps * 10));
}

void MainWindow::open_recent_file()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action == nullptr)
        return;

    if (!ask_to_save_if_modified())
        return;

    try {
        load_file(action->data().toString());
    } catch (const notepad_exception& exception) {
        show_error(exception.what());
    }
}

void MainWindow::create_actions()
{
    saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::save_file);

    for (QAction*& action : recentFileActions) {
        action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::open_recent_file);
    }

    boldAction = new QAction(QIcon(icon_path("bold.svg")), "Bold", this);
    boldAction->setCheckable(true);
    boldAction->setShortcut(QKeySequence::Bold);
    connect(boldAction, &QAction::triggered, this, &MainWindow::apply_bold);

    italicAction = new QAction(QIcon(icon_path("italic.svg")), "Italic", this);
    italicAction->setCheckable(true);
    italicAction->setShortcut(QKeySequence::Italic);
    connect(italicAction, &QAction::triggered, this, &MainWindow::apply_italic);

    underlineAction = new QAction(QIcon(icon_path("underline.svg")), "Underline", this);
    underlineAction->setCheckable(true);
    underlineAction->setShortcut(QKeySequence::Underline);
    connect(underlineAction, &QAction::triggered, this, &MainWindow::apply_underline);

    darkThemeAction = new QAction("Dark Theme", this);
    darkThemeAction->setCheckable(true);
    connect(darkThemeAction, &QAction::triggered, this, &MainWindow::toggle_dark_theme);

    restoreAutosaveAction = new QAction("Restore Autosaved Draft", this);
    restoreAutosaveAction->setEnabled(false);
    connect(restoreAutosaveAction, &QAction::triggered, this, &MainWindow::restore_autosave_draft);

    runPythonAction = new QAction("Run Python", this);
    runPythonAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    connect(runPythonAction, &QAction::triggered, this, &MainWindow::run_python_code);

    stopPythonAction = new QAction("Stop Python", this);
    stopPythonAction->setEnabled(false);
    connect(stopPythonAction, &QAction::triggered, this, &MainWindow::stop_python_code);

    clearPythonConsoleAction = new QAction("Clear Python Console", this);
    connect(clearPythonConsoleAction, &QAction::triggered, this, &MainWindow::clear_python_console);

    showPythonConsoleAction = new QAction("Show Python Console", this);
    showPythonConsoleAction->setCheckable(true);
}

void MainWindow::create_menus()
{
    fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New", QKeySequence::New, this, &MainWindow::new_file);
    fileMenu->addAction("&Open...", QKeySequence::Open, this, &MainWindow::open_file);
    fileMenu->addAction(saveAction);
    fileMenu->addAction("Save &As...", QKeySequence::SaveAs, this, &MainWindow::save_file_as);
    fileMenu->addSeparator();
    fileMenu->addAction("&Print...", QKeySequence::Print, this, &MainWindow::print_document);
    fileMenu->addAction("Export PDF...", this, &MainWindow::export_pdf);
    fileMenu->addAction(restoreAutosaveAction);
    recentFilesMenu = fileMenu->addMenu("Recent Files");
    for (QAction* action : recentFileActions)
        recentFilesMenu->addAction(action);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", QKeySequence::Quit, this, &QWidget::close);

    editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("Undo", QKeySequence::Undo, editor, &QTextEdit::undo);
    editMenu->addAction("Redo", QKeySequence::Redo, editor, &QTextEdit::redo);
    editMenu->addSeparator();
    editMenu->addAction("Cut", QKeySequence::Cut, editor, &QTextEdit::cut);
    editMenu->addAction("Copy", QKeySequence::Copy, editor, &QTextEdit::copy);
    editMenu->addAction("Paste", QKeySequence::Paste, editor, &QTextEdit::paste);
    editMenu->addAction("Select All", QKeySequence::SelectAll, editor, &QTextEdit::selectAll);
    editMenu->addSeparator();
    editMenu->addAction(
        "Find / Replace...", QKeySequence::Find, this, &MainWindow::show_find_replace_dialog);

    QMenu* transformMenu = editMenu->addMenu("Text Case");
    transformMenu->addAction("UPPERCASE", this, &MainWindow::transform_uppercase);
    transformMenu->addAction("lowercase", this, &MainWindow::transform_lowercase);
    transformMenu->addAction("Capitalize Words", this, &MainWindow::transform_capitalize);
    transformMenu->addAction("Sentence case", this, &MainWindow::transform_sentence_case);
    transformMenu->addAction("sWAP cASE", this, &MainWindow::transform_swap_case);

    formatMenu = menuBar()->addMenu("F&ormat");
    formatMenu->addAction(boldAction);
    formatMenu->addAction(italicAction);
    formatMenu->addAction(underlineAction);
    formatMenu->addSeparator();
    formatMenu->addAction("Font...", this, &MainWindow::choose_font);
    formatMenu->addAction("Text Color...", this, &MainWindow::choose_text_color);

    toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction("Check Spelling...", this, &MainWindow::check_spelling);
    toolsMenu->addAction("Word Frequency", this, &MainWindow::show_word_frequency);
    toolsMenu->addSeparator();
    toolsMenu->addAction(runPythonAction);
    toolsMenu->addAction(stopPythonAction);
    toolsMenu->addAction(clearPythonConsoleAction);
    toolsMenu->addAction(showPythonConsoleAction);
    toolsMenu->addSeparator();
    QMenu* syntaxMenu = toolsMenu->addMenu("Syntax Highlighting");
    QActionGroup* syntaxGroup = new QActionGroup(this);
    QAction* plainTextAction = syntaxMenu->addAction("Plain Text");
    QAction* cppAction = syntaxMenu->addAction("C++");
    QAction* pythonAction = syntaxMenu->addAction("Python");

    for (QAction* action : { plainTextAction, cppAction, pythonAction }) {
        action->setCheckable(true);
        syntaxGroup->addAction(action);
    }

    plainTextAction->setChecked(true);

    connect(plainTextAction, &QAction::triggered, this, [this]() {
        spellHighlighter->set_syntax_mode(SpellCheckerHighlighter::SyntaxMode::PlainText);
    });
    connect(cppAction, &QAction::triggered, this,
        [this]() { spellHighlighter->set_syntax_mode(SpellCheckerHighlighter::SyntaxMode::Cpp); });
    connect(pythonAction, &QAction::triggered, this, [this]() {
        spellHighlighter->set_syntax_mode(SpellCheckerHighlighter::SyntaxMode::Python);
    });

    viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("Zoom In", QKeySequence::ZoomIn, this, &MainWindow::zoom_in);
    viewMenu->addAction("Zoom Out", QKeySequence::ZoomOut, this, &MainWindow::zoom_out);
    viewMenu->addAction("Reset Zoom", QKeySequence("Ctrl+0"), this, &MainWindow::reset_zoom);
    viewMenu->addSeparator();
    viewMenu->addAction(darkThemeAction);
}

void MainWindow::create_toolbar()
{
    QToolBar* toolbar = addToolBar("Format");
    toolbar->setObjectName("formatToolbar");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(22, 22));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(boldAction);
    toolbar->addAction(italicAction);
    toolbar->addAction(underlineAction);
    toolbar->addSeparator();
    toolbar->addAction("Font", this, &MainWindow::choose_font);
    toolbar->addAction("Color", this, &MainWindow::choose_text_color);
    toolbar->addSeparator();
    toolbar->addAction("Check Spelling", this, &MainWindow::check_spelling);
    toolbar->addAction(runPythonAction);
}

void MainWindow::create_status_bar()
{
    wordLineLabel = new QLabel(this);
    cursorLabel = new QLabel(this);
    zoomLabel = new QLabel(this);
    autosaveLabel = new QLabel(this);

    statusBar()->addWidget(autosaveLabel);
    statusBar()->addPermanentWidget(wordLineLabel);
    statusBar()->addPermanentWidget(cursorLabel);
    statusBar()->addPermanentWidget(zoomLabel);
}

void MainWindow::create_python_console()
{
    pythonConsole = new QPlainTextEdit(this);
    pythonConsole->setObjectName("pythonConsole");
    pythonConsole->setReadOnly(true);
    pythonConsole->setPlaceholderText("Python output will appear here.");

    pythonConsoleDock = new QDockWidget("Python Console", this);
    pythonConsoleDock->setObjectName("pythonConsoleDock");
    pythonConsoleDock->setWidget(pythonConsole);
    pythonConsoleDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, pythonConsoleDock);
    pythonConsoleDock->hide();

    connect(pythonConsoleDock, &QDockWidget::visibilityChanged, this,
        [this](bool visible) { showPythonConsoleAction->setChecked(visible); });
    connect(
        showPythonConsoleAction, &QAction::triggered, pythonConsoleDock, &QDockWidget::setVisible);
    connect(
        pythonProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::handle_python_stdout);
    connect(
        pythonProcess, &QProcess::readyReadStandardError, this, &MainWindow::handle_python_stderr);
    connect(pythonProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        &MainWindow::handle_python_finished);
    connect(pythonProcess, &QProcess::errorOccurred, this, &MainWindow::handle_python_error);
}

void MainWindow::connect_editor_signals()
{
    connect(
        editor->document(), &QTextDocument::modificationChanged, saveAction, &QAction::setEnabled);
    connect(editor, &QTextEdit::textChanged, this, &MainWindow::handle_text_changed);
    connect(editor, &QTextEdit::cursorPositionChanged, this, &MainWindow::update_status_bar);

    editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(
        editor, &QWidget::customContextMenuRequested, this, &MainWindow::show_editor_context_menu);

    saveAction->setEnabled(false);
}

void MainWindow::setup_autosave()
{
    autosaveTimer->setInterval(5000);
    connect(autosaveTimer, &QTimer::timeout, this, &MainWindow::autosave_draft);
    autosaveTimer->start();
    autosaveLabel->setText("Autosave: idle");
}

void MainWindow::try_restore_autosave()
{
    if (!autosaveManager.has_draft())
        return;

    restoreAutosaveAction->setEnabled(true);
    autosaveLabel->setText("Autosave: draft available");
    statusBar()->showMessage("Recovered draft available in File > Restore Autosaved Draft.", 7000);
}

void MainWindow::load_spell_checker()
{
    try {
        spellChecker.load_words("data/words.txt");
    } catch (const notepad_exception& exception) {
        show_error(exception.what());
    }
}

void MainWindow::load_file(const QString& path)
{
    QFileInfo info(path);
    if (!info.exists())
        throw file_not_found_exception(path.toStdString());

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw file_read_exception(path.toStdString());

    QTextStream stream(&file);
    QString content = stream.readAll();

    if (Qt::mightBeRichText(content))
        editor->setHtml(content);
    else
        editor->setPlainText(content);

    currentFilePath = path;
    editor->document()->setModified(false);
    add_recent_file(path);
    update_title();
    clear_autosave();
    spellHighlighter->rehighlight();
}

void MainWindow::write_file(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw file_write_exception(path.toStdString());

    QTextStream stream(&file);
    QString suffix = QFileInfo(path).suffix().toLower();

    if (suffix == "html" || suffix == "htm")
        stream << editor->toHtml();
    else
        stream << editor->toPlainText();

    editor->document()->setModified(false);
    clear_autosave();
}

bool MainWindow::ask_to_save_if_modified()
{
    if (!editor->document()->isModified())
        return true;

    QMessageBox::StandardButton answer
        = QMessageBox::warning(this, "Notepad", "The document has unsaved changes.",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (answer == QMessageBox::Save) {
        save_file();
        return !editor->document()->isModified();
    }

    if (answer == QMessageBox::Discard) {
        clear_autosave();
        return true;
    }

    return false;
}

void MainWindow::show_error(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::add_recent_file(const QString& path)
{
    QSettings settings;
    QStringList files = settings.value(recent_files_key()).toStringList();

    files.removeAll(path);
    files.prepend(path);

    while (files.size() > maximumRecentFiles)
        files.removeLast();

    settings.setValue(recent_files_key(), files);
    update_recent_files_menu();
}

void MainWindow::update_recent_files_menu()
{
    QSettings settings;
    QStringList files = settings.value(recent_files_key()).toStringList();

    int count = qMin(files.size(), maximumRecentFiles);
    for (int i = 0; i < maximumRecentFiles; ++i) {
        if (i < count) {
            QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
            recentFileActions[i]->setText(text);
            recentFileActions[i]->setData(files[i]);
            recentFileActions[i]->setVisible(true);
        } else {
            recentFileActions[i]->setVisible(false);
        }
    }

    recentFilesMenu->setEnabled(count > 0);
}

void MainWindow::clear_autosave()
{
    autosaveManager.clear();
    if (restoreAutosaveAction != nullptr)
        restoreAutosaveAction->setEnabled(false);
    if (autosaveLabel != nullptr)
        autosaveLabel->setText("Autosave: idle");
}

void MainWindow::apply_theme(bool darkMode)
{
    darkThemeAction->setChecked(darkMode);

    if (!darkMode) {
        qApp->setStyleSheet(R"(
            QMainWindow {
                background: #f6f7f9;
            }
            QToolBar#formatToolbar {
                background: #ffffff;
                border: 0;
                border-bottom: 1px solid #dde1e7;
                spacing: 6px;
                padding: 10px 12px;
            }
            QToolBar#formatToolbar QToolButton {
                background: transparent;
                color: #24292f;
                border: 1px solid transparent;
                border-radius: 7px;
                padding: 7px 10px;
                font-weight: 600;
            }
            QToolBar#formatToolbar QToolButton:hover {
                background: #eef2f7;
                border-color: #d7dde5;
            }
            QToolBar#formatToolbar QToolButton:checked {
                background: #dbeafe;
                border-color: #93c5fd;
                color: #1d4ed8;
            }
            QToolBar::separator {
                background: #d8dee6;
                width: 1px;
                margin: 4px 8px;
            }
            QTextEdit {
                background: #ffffff;
                color: #1f2328;
                selection-background-color: #2563eb;
                selection-color: #ffffff;
                border: 0;
            }
            QPlainTextEdit {
                background: #0f172a;
                color: #e2e8f0;
                selection-background-color: #2563eb;
                border: 0;
                padding: 10px;
                font-family: Menlo, Consolas, monospace;
            }
            QDockWidget {
                background: #ffffff;
                color: #24292f;
                titlebar-close-icon: none;
            }
            QStatusBar {
                background: #ffffff;
                color: #57606a;
                border-top: 1px solid #dde1e7;
                padding: 4px 10px;
            }
            QStatusBar QLabel {
                color: #57606a;
                padding: 0 6px;
            }
            QMenu {
                background: #ffffff;
                color: #24292f;
                border: 1px solid #d0d7de;
                padding: 6px;
            }
            QMenu::item {
                border-radius: 6px;
                padding: 6px 22px;
            }
            QMenu::item:selected {
                background: #eef2ff;
                color: #1d4ed8;
            }
            QDialog, QTableWidget, QLineEdit {
                background: #ffffff;
                color: #24292f;
            }
            QPushButton {
                background: #f6f8fa;
                color: #24292f;
                border: 1px solid #d0d7de;
                border-radius: 7px;
                padding: 6px 12px;
            }
            QPushButton:hover {
                background: #eef2f7;
            }
            QHeaderView::section {
                background: #f6f8fa;
                color: #57606a;
                border: 0;
                border-bottom: 1px solid #d0d7de;
                padding: 6px;
            }
        )");
        return;
    }

    qApp->setStyleSheet(R"(
        QMainWindow {
            background: #111315;
        }
        QToolBar#formatToolbar {
            background: #25282c;
            color: #f1f3f4;
            border: 0;
            border-bottom: 1px solid #3a3f45;
            spacing: 6px;
            padding: 10px 12px;
        }
        QToolBar#formatToolbar QToolButton {
            background: transparent;
            color: #e8eaed;
            border: 1px solid transparent;
            border-radius: 7px;
            padding: 7px 10px;
            font-weight: 600;
        }
        QToolBar#formatToolbar QToolButton:hover {
            background: #33373d;
            border-color: #4a5058;
        }
        QToolBar#formatToolbar QToolButton:checked {
            background: #1e3a5f;
            border-color: #3b82f6;
            color: #dbeafe;
        }
        QToolBar::separator {
            background: #3a3f45;
            width: 1px;
            margin: 4px 8px;
        }
        QTextEdit {
            background: #181a1d;
            color: #f8f9fa;
            selection-background-color: #3b82f6;
            selection-color: #ffffff;
            border: 0;
        }
        QPlainTextEdit {
            background: #0b1020;
            color: #dbeafe;
            selection-background-color: #3b82f6;
            border: 0;
            padding: 10px;
            font-family: Menlo, Consolas, monospace;
        }
        QDockWidget {
            background: #25282c;
            color: #f1f3f4;
            titlebar-close-icon: none;
        }
        QStatusBar {
            background: #25282c;
            color: #c9d1d9;
            border-top: 1px solid #3a3f45;
            padding: 4px 10px;
        }
        QStatusBar QLabel {
            color: #c9d1d9;
            padding: 0 6px;
        }
        QMenu {
            background: #25282c;
            color: #f1f3f4;
            border: 1px solid #3a3f45;
            padding: 6px;
        }
        QMenu::item {
            border-radius: 6px;
            padding: 6px 22px;
        }
        QMenu::item:selected {
            background: #3b82f6;
            color: #ffffff;
        }
        QDialog, QLabel, QTableWidget, QLineEdit, QPushButton {
            background: #25282c;
            color: #f1f3f4;
        }
        QPushButton {
            border: 1px solid #4a5058;
            border-radius: 7px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background: #33373d;
        }
        QHeaderView::section {
            background: #33373d;
            color: #f1f3f4;
            border: 0;
            border-bottom: 1px solid #4a5058;
            padding: 6px;
        }
    )");
}

void MainWindow::merge_format_on_selection(const QTextCharFormat& format)
{
    QTextCursor cursor = editor->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    cursor.mergeCharFormat(format);
    editor->mergeCurrentCharFormat(format);
}

void MainWindow::apply_transform(QString (*transform)(QString))
{
    QTextCursor cursor = editor->textCursor();
    bool hadSelection = cursor.hasSelection();

    if (!hadSelection)
        cursor.select(QTextCursor::Document);

    QString selectedText = cursor.selectedText();
    selectedText.replace(QChar::ParagraphSeparator, "\n");
    cursor.insertText(transform(selectedText));

    if (!hadSelection)
        cursor.clearSelection();

    editor->setTextCursor(cursor);
}

void MainWindow::handle_text_changed()
{
    cachedWordCount = calculate_word_count();
    update_status_bar();
}

int MainWindow::calculate_word_count() const
{
    int words = 0;
    static const QRegularExpression wordExpression("[A-Za-z]+");
    QRegularExpressionMatchIterator iterator = wordExpression.globalMatch(editor->toPlainText());

    while (iterator.hasNext()) {
        iterator.next();
        ++words;
    }

    return words;
}

std::vector<std::pair<QString, int>> MainWindow::collect_word_frequency() const
{
    QMap<QString, int> counts;
    static const QRegularExpression wordExpression("[A-Za-z]+");
    QRegularExpressionMatchIterator iterator
        = wordExpression.globalMatch(editor->toPlainText().toLower());

    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        counts[match.captured()]++;
    }

    std::vector<std::pair<QString, int>> result;
    for (auto iterator = counts.constBegin(); iterator != counts.constEnd(); ++iterator)
        result.push_back({ iterator.key(), iterator.value() });

    std::sort(result.begin(), result.end(), word_frequency_less);
    return result;
}

QString MainWindow::preserve_case(const QString& original, const QString& suggestion) const
{
    if (original == original.toUpper())
        return suggestion.toUpper();

    if (!original.isEmpty() && original.front().isUpper()) {
        QString result = suggestion.toLower();
        result[0] = result[0].toUpper();
        return result;
    }

    return suggestion;
}
