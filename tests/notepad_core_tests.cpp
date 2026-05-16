#include "autosave_manager.h"
#include "notepad_exception.h"
#include "spell_checker.h"
#include "text_transform.h"

#include <QTemporaryDir>

#include <cassert>
#include <fstream>

int main()
{
    assert(text_transform::uppercase("Hello") == "HELLO");
    assert(text_transform::lowercase("Hello") == "hello");
    assert(text_transform::capitalize("hello WORLD") == "Hello World");
    assert(text_transform::sentence_case("hELLO. wORLD!") == "Hello. World!");
    assert(text_transform::sentence_case("  one? TWO. three") == "  One? Two. Three");
    assert(text_transform::swap_case("AbC") == "aBc");
    assert(SpellChecker::normalize_word("Hello, WORLD!") == "helloworld");
    assert(SpellChecker::normalize_word("can't") == "cant");

    QTemporaryDir directory;
    std::string dictionaryPath = (directory.path() + "/words.txt").toStdString();
    QString autosavePath = directory.path() + "/autosave.html";

    std::ofstream dictionary(dictionaryPath);
    dictionary << "hello\nworld\nproject\nnotepad\nspelling\n";
    dictionary.close();

    SpellChecker checker;
    checker.load_words(dictionaryPath);

    assert(checker.is_loaded());
    assert(checker.is_correct("Hello"));
    assert(checker.is_correct("world!"));
    assert(!checker.is_correct("wurld"));

    std::vector<std::string> suggestions = checker.suggestions("wurld", 2);
    assert(!suggestions.empty());
    assert(suggestions.size() <= 2);
    assert(suggestions.front() == "world");

    std::vector<std::string> firstLetterTypoSuggestions = checker.suggestions("xorld");
    assert(!firstLetterTypoSuggestions.empty());
    assert(firstLetterTypoSuggestions.front() == "world");

    bool missingDictionaryThrows = false;
    try {
        checker.load_words(directory.path().toStdString() + "/missing.txt");
    } catch (const file_read_exception&) {
        missingDictionaryThrows = true;
    }
    assert(missingDictionaryThrows);

    AutosaveManager autosave(autosavePath);
    assert(!autosave.has_draft());
    assert(autosave.write_html("<p>draft</p>"));
    assert(autosave.has_draft());
    bool readOk = false;
    assert(autosave.read_html(&readOk).contains("draft"));
    assert(readOk);
    autosave.clear();
    assert(!autosave.has_draft());

    return 0;
}
