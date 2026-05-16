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

    bool missingDictionaryThrows = false;
    try {
        checker.load_words(directory.path().toStdString() + "/missing.txt");
    } catch (const file_read_exception&) {
        missingDictionaryThrows = true;
    }
    assert(missingDictionaryThrows);

    return 0;
}
