# Notepad Project Report

## Overview

This project extends the required Qt Notepad application into a small WordPad-style editor. It keeps the original notepad behavior, adds the required exception handling and spell checker, and includes more than three optional features for bonus-quality work.

## Required Feature 1: Exception Handling

The file `notepad_exception.h` defines the required exception hierarchy:

- `notepad_exception`
- `file_not_found_exception`
- `file_read_exception`
- `file_write_exception`

`MainWindow::open_file()` and `MainWindow::save_file()` call helper functions that throw these exceptions when file operations fail. The public slots catch `notepad_exception` and show `QMessageBox::critical` with the exact title `Error`.

## Required Feature 2: Spell Checker

The spell checker is implemented in `spell_checker.h`, `spell_checker.cpp`, `spell_checker_highlighter.h`, and `spell_checker_highlighter.cpp`.

- `SpellChecker::load_words()` loads `data/words.txt` into a `std::set<std::string>`.
- Words are normalized by lowercasing and removing non-alphabetic characters.
- `SpellCheckerHighlighter` is a `QSyntaxHighlighter` subclass that underlines misspelled words with `QTextCharFormat::SpellCheckUnderline` in red.
- The editor has a right-click context menu that shows up to five spelling suggestions for a misspelled word.
- `Tools > Check Spelling...` re-runs highlighting for the whole document.

## Optional Features Implemented

1. Cursor line and column indicator: the status bar shows the current line and column.
2. Font dialog: `Format > Font...` opens `QFontDialog` and applies the selected font.
3. Color picker: `Format > Text Color...` opens `QColorDialog`.
4. Print: `File > Print...` opens `QPrintDialog` and prints through `QTextEdit::print()`.
5. Recent files: `File > Recent Files` keeps the last five files using `QSettings`.
6. Line numbers: `TextEdit` displays a line-number margin beside the editor.
7. Syntax highlighting: `Tools > Syntax Highlighting` supports Plain Text, C++, and Python keyword highlighting.
8. Zoom: `View > Zoom In`, `Zoom Out`, and `Reset Zoom` support standard shortcuts.

## Extra Bonus Features

- PDF export: `File > Export PDF...` saves the current document as a PDF.
- Dark theme: `View > Dark Theme` switches the editor into a persistent dark mode.
- Autosave and recovery: unsaved work is periodically saved to an application data draft. If a draft exists, `File > Restore Autosaved Draft` becomes available without blocking the main window.
- Python runner: `Tools > Run Python` runs the selected Python code, or the whole document if nothing is selected, and shows output in a bottom `Python Console` panel. `Tools > Stop Python` stops a running script, and `Tools > Clear Python Console` clears the output.

## Demo Checklist

During the demo, the strongest flow is:

1. Type a misspelled word such as `wurld` and show the red underline.
2. Right-click the misspelled word and choose a suggestion.
3. Open `Tools > Syntax Highlighting > C++` or `Python` and type a short code snippet to show keyword highlighting.
4. Use `Format > Font...`, `Format > Text Color...`, bold, italic, and underline on selected text.
5. Open `Edit > Find / Replace...` and replace a word.
6. Open `Tools > Word Frequency` to show the frequency dialog.
7. Use `View > Dark Theme`, zoom controls, and the line/column status bar.
8. Use `File > Export PDF...` or `File > Print...` to show output features.
9. Type `print(40 + 2)`, choose `Tools > Run Python`, and show the `Python Console` output.

## Existing Features Preserved

The application still supports:

- new, open, save, save as, and exit
- undo, redo, cut, copy, paste, and select all
- uppercase, lowercase, capitalize, sentence case, and swap case
- bold, italic, underline, font selection, and text color
- find and replace
- word frequency analysis
- live word count, line count, cursor position, and zoom status

## Design Notes

`MainWindow` owns the main GUI and coordinates actions. Spell checking is separated into `SpellChecker` for dictionary logic and `SpellCheckerHighlighter` for editor highlighting. `TextEdit` subclasses `QTextEdit` only for line-number painting, so rich text editing still works normally.

`SpellCheckerHighlighter` also contains the optional syntax highlighter. This keeps all document highlighting in one `QSyntaxHighlighter` subclass, which avoids conflicts between multiple highlighters on the same `QTextDocument`.

`AutosaveManager` owns draft-file reading, writing, and cleanup so that recovery logic is not mixed directly with menu-building code. `SpellChecker` keeps both a `std::set` for exact lookup and a first-letter index for faster suggestions. The status bar caches the word count and recalculates it only when the document text changes, while cursor movement only refreshes line and column.

Screenshots are intentionally not committed because the assignment warns against unrelated extra files. The demo checklist above documents the exact features to show during the live grading session.

The project builds with:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

GitHub Actions runs the same build and test steps on every push.
