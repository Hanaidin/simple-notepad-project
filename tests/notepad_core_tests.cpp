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
    assert(text_transform::swap_case("AbC") == "aBc");

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

    std::vector<std::string> suggestions = checker.suggestions("wurld");
    assert(!suggestions.empty());
    assert(suggestions.front() == "world");

    return 0;
}
