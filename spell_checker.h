#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H

#include <map>
#include <set>
#include <string>
#include <vector>

class SpellChecker {
public:
    void load_words(const std::string& path);

    bool is_loaded() const;
    bool is_correct(const std::string& word) const;
    std::vector<std::string> suggestions(const std::string& word, int limit = 5) const;

    static std::string normalize_word(const std::string& word);

private:
    static int edit_distance_limited(const std::string& left, const std::string& right, int limit);

    std::set<std::string> words;
    std::map<char, std::vector<std::string>> wordsByFirstLetter;
};

#endif
