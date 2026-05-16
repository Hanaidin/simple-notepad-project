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
7. Zoom: `View > Zoom In`, `Zoom Out`, and `Reset Zoom` support standard shortcuts.

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

The project builds with:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

GitHub Actions runs the same build and test steps on every push.
